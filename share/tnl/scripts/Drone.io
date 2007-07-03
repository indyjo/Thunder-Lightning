Drone do(
    TRAVEL_SPEED := 400 / 3.6
    TRAVEL_HEIGHT := 500

    init := method(
        self command_queue := CommandQueue clone
    )
    
    flightState := coro(me,
        self gravity := vector(0,-9.81,0)

        self p := me getLocation
        self v := me getMovementVector
        self orient := me getOrientation
        self gravity_lcs := orient * gravity
        self right := orient * vector(1,0,0)
        self up := orient * vector(0,1,0)
        self front := orient * vector(0,0,1)
        self orient_inv := orient transpose
        
        self altitude := p at(1)
        self height := altitude - Terrain heightAt(p(0), p(2))
        
        self accel := vector(0,0,0)
        self perceived_accel := accel - gravity
        self accel_lcs := orient_inv * accel
        self perceived_accel_lcs := orient_inv * perceived_accel
        
        dt := 0
        loop(
            dt = pass

            accel = me getMovementVector - v
            
            p = me getLocation()
            v = me getMovementVector()
            orient = me getOrientation
            gravity_lcs = orient * gravity
            right = orient * vector(1,0,0)
            up = orient * vector(0,1,0)
            front = orient * vector(0,0,1)
            orient_inv = orient transpose
            
            altitude = p at(1)
            height = altitude - Terrain heightAt(p(0), p(2))
            
            perceived_accel = accel - gravity
            accel_lcs = orient_inv * accel
            perceived_accel_lcs = orient_inv * perceived_accel
        )
    )
    
    accelerateXWithRudder := coro(me, arg_target_accel,
        if((self ?target_accel) isNil, self target_accel := arg_target_accel)

        self error := target_accel - me state accel at(0)
        new_error := 0
        
        dt := 0
        derivative := 0
        integral := 0
        
        loop (
            dt = pass
            new_error = target_accel - me state accel_lcs at(0)
            derivative = (new_error - error) / dt
            integral = (integral + new_error * dt) clip(-5,5)
            error = new_error
            #"accel: #{me state accel at(1)} error: #{error} deriv: #{derivative} int: #{integral}" interpolate say
            
            me controls setFloat("rudder",
                (0.01*error + 0.003*integral + 0.01*derivative) clip(-1,1))
        )
    )
    
    accelerateYWithElevator := coro(me, arg_target_accel,
        if((self ?target_accel) isNil, self target_accel := arg_target_accel)
        
        self error := target_accel - me state accel at(1)
        new_error := 0
        
        dt := 0
        derivative := 0
        integral := 0
        
        loop (
            dt = pass
            new_error = target_accel - me state accel_lcs at(1)
            derivative = (new_error - error) / dt
            integral = (integral + new_error * dt) clip(-5,5)
            error = new_error
            #"accel: #{me state accel at(1)} error: #{error} deriv: #{derivative} int: #{integral}" interpolate say
            
            if (me state v len > 220/3.6,
                me controls setFloat("elevator",
                    -(0.015*error + 0.002*integral + 0.015*derivative) clip(-1,1))
            ,
                me controls setFloat("elevator",
                    -(0.04*error + 0.01*integral + 0.06*derivative) clip(-1,1))
            )
            
        )
    )
    
    accelerateZWithThrottle := coro(me, arg_target_accel,
        if((self ?target_accel) isNil, self target_accel := arg_target_accel)

        self error := target_accel - me state accel_lcs at(2)
        new_error := 0
        
        dt := 0
        derivative := 0
        integral := 0
        
        loop (
            dt = pass
            new_error = target_accel - me state accel_lcs at(2)
            #derivative = (new_error - error) / dt
            integral = (integral + new_error * dt) clip(0,10)
            error = new_error
            #"accel: #{me state accel at(2)} error: #{error} int: #{integral}" interpolate say
            
            me controls setFloat("throttle",
                (0.05*error + 0.05*integral) clip(0,1))
        )
    )
    
    accelerateLCS := coro(me, arg_target_accel,
        if((self ?target_accel) isNil, self target_accel := arg_target_accel)
        
        // forward to elevator, rudder and throttle controllers
        self x := me accelerateXWithRudder clone start(me)
        manage(x)
        self y := me accelerateYWithElevator clone start(me)
        manage(y)
        self z := me accelerateZWithThrottle clone start(me)
        manage(z)
        
        loop(
            x target_accel := target_accel at(0)
            y target_accel := target_accel at(1)
            z target_accel := target_accel at(2)
            pass
        )
    )

    accelerateWCS := coro(me, arg_target_accel,
        if((self ?target_accel) isNil, self target_accel := arg_target_accel)
        
        // forward to local coordinate space
        self lcs := me accelerateLCS clone start(me)
        manage(lcs)
        
        loop(
            lcs target_accel := me state orient_inv * target_accel
            pass
        )
    )

    supportAccelWithAileronWCS := coro(me, arg_target_accel,
        target_axis := target_accel - me state gravity
        error := target_axis dot(me state right)
        new_error := 0
        loop(
            dt := pass
            target_axis = target_accel - me state gravity
            new_error = target_axis dot(me state right)
            #new_error := (target_accel - me state gravity_lcs) at(0)
            derivative := (new_error - error) / dt
            error := new_error
            #"error: #{error} deriv: #{derivative}" interpolate say
            me controls setFloat("aileron", (0.015*error + 0.03*derivative) clip(-1,1))
        )
    )
    
    balancePerceivedAccelWithAileron := coro(me,
        error := me state perceived_accel_lcs at(0)
        loop(
            dt := pass
            new_error := me state perceived_accel_lcs at(0)
            derivative := (new_error - error) / dt
            error := new_error
            me controls setFloat("aileron", (0.15*error + 0.05*derivative) clip(-1,1))
        )
    )
    
    flyVector := coro(me, arg_target_vector,
        if((self ?target_vector) isNil, self target_vector := arg_target_vector)
        
        self accel := Drone accelerateWCS clone start(me)
        manage(accel)
        
        self aileron := Drone supportAccelWithAileronWCS clone start(me)
        manage(aileron)
        
        error := target_vector - me state v
        accel target_accel := vector(0,0,0)
        aileron target_accel := vector(0,0,0)
        target_accel := vector(0,0,0)
        
        loop(
            dt := pass
            mod_target_vector := target_vector
            if (mod_target_vector dot(me state front) < 0,
                mod_target_vector = mod_target_vector - me state front * mod_target_vector dot(me state front)
                mod_target_vector = mod_target_vector * (target_vector len / mod_target_vector len)
            )
            new_error := mod_target_vector - me state v
            derivative := (new_error - error) * (1/dt)
            error = new_error
            
            target_accel = 0.4*error+0.15*derivative
            accel target_accel = target_accel
            aileron target_accel = target_accel
            #"error: #{error}" interpolate say
        )
    )
    
    maintainPosition := coro(me, arg_mps_per_m, arg_target_position, arg_target_vector,
        if((self ?mps_per_m) isNil, self mps_per_m := arg_mps_per_m ifNilEval(0.2))
        if((self ?target_position) isNil, self target_position := arg_target_position)
        if((self ?target_vector) isNil, self target_vector := arg_target_vector)
        
        max_dv := 300 / 3.6
        
        fly := Drone flyVector clone start(me, target_vector)
        manage(fly)
        
        loop(
            delta_v := (target_position - me state p)*mps_per_m
            if (delta_v len > max_dv, delta_v = delta_v * (max_dv / delta_v len))
            
            fly target_vector := target_vector + delta_v
            pass
        )
    )
    
    followSegment := coro(me, arg_a, arg_b, arg_v,
        if((self ?a) isNil, self a := arg_a)
        if((self ?b) isNil, self b := arg_b)
        if((self ?v) isNil, self v := arg_v)
        
        
        mp := Drone maintainPosition clone start(me)
        manage(mp)
        loop(
            ab_norm := (b-a) norm
            mp target_vector := v * ab_norm
            mp target_position := a + ab_norm * (ab_norm dot(me state p - a))
            if (?debug,
                "Target pos: #{mp target_position} err: #{(mp target_position - me state p) len}" interpolate say
            )
            pass
        )
    )
    
    followPath := coro(me, navpath, v,
        self fs := Drone followSegment clone start(me)
        manage(fs)
        
        fs v := v
        
        while (navpath size > 1,
            segment := navpath currentSegmentSmoothed(me location)
            fs a := segment at(0)
            fs b := segment at(1)
            pass
        )
    )
    
    safety_height := vector(0,5,0)
    descent_angle := 10*Number constants pi / 180
    descent_speed := 150 / 3.6
    final_length := 1000
    
    returnToFinal := coro(me, rwy, dangle, final,
        height := final * dangle sin
        a := rwy runwayBegin + Drone safety_height
        b := rwy runwayEnd + Drone safety_height
        d := (b-a) atSet(1,0) norm
        up := vector(0,1,0)
        right := up % d
        p0 := me state p
        p1 := b + 0.3*final*d + height*up
        p2 := p1 + right*final*0.3
        p3 := a - d*final + up*height + right*final*0.3
        p4 := a - d*final + up*height + right*final*0.1
        
        self fp := Drone followPath clone start(me, NavPath clone with(list(p0,p1,p2,p3,p4)), 350/3.6)
        manage(fp)
        
        while( fp running,
            pass
        )
    )
    
    finalApproach := coro(me, rwy, dangle,
        //"Beginning final approach" say
        a := rwy runwayBegin + Drone safety_height
        b := rwy runwayEnd + Drone safety_height
        p0 := a + (a-b) atSet(1,0) norm * dangle cos + vector(0,dangle sin, 0)
        #"p0: #{p0} a:#{a}" interpolate println
        
        dspeed := Drone descent_speed
        
        fs :=  Drone followSegment clone start(me, p0,a, dspeed)
        manage(fs)
        #fs debug:= true
        
        while ( ((me state p) - a) len > 100,
            pass
        )
        
        # safety checks
        if (me state v len between(dspeed - 8/3.6,dspeed + 4/3.6) not,
            //"Your speed doesn't look good" say
            return
        )
        if (rwy isRunwayFree not,
            //"Runway isn't free" say
            return
        )
        d := p0 - a
        p_proj := a + d*(d dot(me state p - a) )
        if ((p_proj - me state p) len > 2.5,
            //"You are to far away from glide slope" say
            return
        )
        
        while( (a-b) dot (me state p - a) > 0,
            pass
        )
        
        fs interrupt
        me controls setFloat("brake", 1)
        me controls setFloat("aileron", 0)
        me controls setFloat("elevator", 0)
        me controls setFloat("rudder", 0)
        me controls setFloat("throttle", 0)
        
        while( me state v len > 0.5,
            pass
        )
        
        self status := "LANDED"
    )
        
        
    waitForLandingClearance := coro(me, rwy,
        if (rwy isRunwayLocked not,
            rwy lockRunway(me)
            return
        )
        height := rwy requestHoldingLevel(me)
        "Holding at level #{height}" interpolate println
        
        a := rwy runwayBegin + Drone safety_height
        b := rwy runwayEnd + Drone safety_height
        d := (b-a) atSet(1,0) norm
        up := vector(0,1,0)
        left := (up % d) * -1
        
        self fp := Drone followPath clone
        manage(fp)
        loop (
            if ( fp running not,
                if (rwy isRunwayLocked not,
                    rwy releaseHoldingLevel(me)
                    rwy lockRunway(me)
                    return
                )
                waypoints := list
                n := 64
                n repeat(i,
                    alpha := 2*Number constants pi * i / n
                    wpt := a + up*height + 2000 * left + 1000 * vector(alpha cos, 0, - alpha sin)
                    #wpt println
                    waypoints append( wpt )
                )
                fp start(me, NavPath clone with(waypoints), 350/3.6)
            )
            pass
        )
    )
        
    
    performLanding := coro(me, rwy, get_eaten,
        # descent angle used to calculate begin of final
        dangle := Drone descent_angle
        final := Drone final_length
        height := final * dangle sin
        a := rwy runwayBegin + Drone safety_height
        b := rwy runwayEnd + Drone safety_height
        d := (b-a) atSet(1,0) norm
        up := vector(0,1,0)
        right := up % d
        
        
        self hold := Drone waitForLandingClearance clone start(me, rwy)
        manage(hold)
        while (hold running, pass)
        
        final_begin := a - d*final + up*height
        
        "Proceeding to final" say
        self to_final := Drone followPath clone start(me,
            NavPath clone with(list(me state p, final_begin - 600 * right, final_begin)),
            250/3.6)
        manage(to_final)
        
        while ((me state p - final_begin) len > 300,
            pass
        )
        to_final interrupt
        
        loop(
            self fa := Drone finalApproach clone start(me, rwy, dangle)
            manage(fa)
            me controls setBool("landing_gear", true)
            me controls setBool("landing_hook", true)
            
            while (fa running,
                pass
            )
            
            if ((fa ?status) == "LANDED",
                me setControlMode(Actor UNCONTROLLED)
                if (get_eaten and rwy droneEatable(me),
                    rwy eatDrone(me)
                )
                break
                #we're done!
            )
            
            # else let's play a little safer this time
            final = final + 200
            #"Returning to final" say
            self ret := Drone returnToFinal clone start(me, rwy, dangle, final)
            manage(ret)
            me controls setBool("landing_gear", false)
            while (ret running,
                pass
            )
        )
    )
    
    land := coro(me, rwy, get_eaten,
        ex := try(
            manage( me performLanding clone start(me, rwy, get_eaten) )
            loop(pass)
        )
        
        if(rwy isRunwayLockedTo(me), rwy unlockRunway)
        ex ifNonNil(ex raise)
    )
            
                
    takeoff := coro(me, runway,
        me controls setBool("landing_hook", false)
        me controls setBool("landing_gear", true)
        
        me controls setFloat("aileron", 0)
        me controls setFloat("elevator", -0.2)
        me controls setFloat("rudder", 0)
        me controls setFloat("throttle", 0)
        
        sleep(5)
        me controls setFloat("throttle", 1)
        sleep(4)
        me controls setFloat("elevator", 0)
        me controls setBool("landing_gear", false)
    )
    
    flyInFormation := coro(me, arg_partner, arg_pos,
        if((self ?partner) isNil, self partner := arg_partner)
        if((self ?pos) isNil, self pos := arg_pos)
        
        self mp := Drone maintainPosition clone start(me, 0.5)
        manage(mp)
        
        while(partner isAlive,
            mp target_vector := partner getMovementVector
            mp target_position := partner getLocation + partner getOrientation * pos
            pass
        )
        
        partner = nil
    )
    
    travelTo := coro(me, target,
        dist := (me location2 - target) length
        segment_length := 100
        n_segments := (dist / segment_length) ceil
        segment_length = dist / n_segments
        
        step := (target - me location2)
        
        p := me location2
        path := list
        (n_segments + 1) repeat(
            path append( vector(p at(0), Terrain heightAt(p at(0), p at(1)) + me TRAVEL_HEIGHT, p at(1) ) )
            p = p + step
        )
        
        navpath := NavPath clone with(path)
        writeln("Navpath: ", navpath)
        manage ( me followPath clone start(me, navpath, me TRAVEL_SPEED) )
        loop(pass)
    )
    
    adHocCommand := method(
        azimuth := 2 * Number constants pi * Random value
        dist := 2000 + 2000 * Random value
        
        p := me location2 + vector( azimuth sin, azimuth cos ) * dist
        Command Goto clone with (p, 500)
    )
    
    executeCommand := coro(me, command,
        command action switch(
            Command GOTO, do(
                manage( me travelTo clone start(me, command argVec2) )
                loop(pass)
            ),
            Command PATH, do(
                manage( me followPath clone start(
                    me,
                    command argPath,
                    command argFloat ifNilEval(me TRAVEL_SPEED)
                ) )
                loop(pass)
            ),
            Command LAND, do(
                rwy := command argActor link
                get_eaten := command argBool
                if (rwy and rwy isAlive,
                    #"Performing landing." say
                    manage( me land clone start(me, rwy, get_eaten) )
                    loop(pass)
                ,
                    "The runway doesn't exist anymore." say
                )
                
            ),
            Command TAKEOFF, do(
                rwy := command argActor link
                if (rwy and rwy isAlive,
                    #"Performing takeoff." say
                    manage( me takeoff clone start(me, rwy) )
                    loop(pass)
                ,
                    "The runway doesn't exist anymore." say
                )
                
            ),
            ("Command " .. command action .. " not implemented yet.") say
        )
    )
    
    executeCommandQueue := coro(me,
        command := nil
        handler := nil
        
        loop(
            c := me command_queue currentCommand(me)
            if ( c != command,
                #("New command: " .. c action) say
                command = c
                handler ifNonNil(
                    handler interrupt
                )
                
                c ifNonNil(
                    handler = Drone executeCommand clone start(me, command)
                    manage(handler)
                )
                
                c ifNil(
                    me command_queue setFallbackCommand(me adHocCommand)
                )
            )
            
            sleep(0.5)
        )
    )
    
    ai := coro(me,
        #self fly := Drone followSegment clone start(me, vector(0,1500,0), vector(0,1500,1), v)
        #manage(fly)
        manage (me executeCommandQueue clone start(me))

        loop(
            pass
        )
    )
    
    on("start_ai",
        # ai depends on flightState. Io will run coros in LIFO order, so it seems
        # to be ok this way.
        self _ai := ai clone start(self)
        self state := flightState clone start(self)
        
        ("AI of Drone ".. self uniqueHexId .. " started") println
    )
    on("stop_ai",
        _ai interrupt
        state interrupt
        
        ("AI of Drone ".. self uniqueHexId .. " interrupted") println
    )
)

