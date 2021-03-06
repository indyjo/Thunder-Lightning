Drone coro := method(
    res := resend
    res tag := "Drone"
    res
)


doFile(Config scripts_dir .. "/Drone_attack.io")

Drone do(
    appendProto(CommonAI)
    
    TRAVEL_SPEED := 400 / 3.6
    ATTACK_SPEED := 500 / 3.6
    TRAVEL_HEIGHT := 500

    init := method(
        self command_queue := CommandQueue clone
    )
    
    isAirborneTarget := method(
        velocity length > 36
    )
    isGroundTarget := method(
        velocity length <= 36
    )
    
    FlightState := coro(me,
        self gravity := vector(0,-9.81,0)

        self p := me getLocation
        self v := me getMovementVector
        self orient := me getOrientation
        self gravity_lcs := orient * gravity
        self right := orient col(0)
        self up := orient col(1)
        self front := orient col(2)
        self orient_inv := orient transpose
        self dir := if (v lenSquare > 0.0000001, v norm, front)
        
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
            gravity_lcs = orient matMult(gravity)
            right = orient col(0)
            up = orient col(1)
            front = orient col(2)
            orient_inv = orient transpose
            dir = if (v lenSquare > 0.0000001, v norm, front)
            
            altitude = p y
            height = altitude - Terrain heightAt(p x, p z)
            
            //perceived_accel = accel - gravity
            accel_lcs = orient_inv matMult(accel)
            //perceived_accel_lcs = orient_inv matMult(perceived_accel)
        )
    )
    
    AccelerateXWithRudder := coro(me, arg_target_accel,
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
    
    AccelerateYWithElevator := coro(me, arg_target_accel,
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
    
    AccelerateZWithThrottle := coro(me, arg_target_accel,
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
            integral = (integral + new_error * dt) clip(-5,100)
            error = new_error
            #if (me == Game viewSubject,
            #    "accel: #{me state accel at(2) asString(2)} error: #{error asString(2)} int: #{integral asString(2)}" interpolate say
            #)
            
            me controls setFloat("throttle",
                (0.05*error + 0.01*integral) clip(0,1))
        )
    )
    
    AccelerateLCS := coro(me, arg_target_accel,
        if((self ?target_accel) isNil, self target_accel := arg_target_accel)
        
        // forward to elevator, rudder and throttle controllers
        self x := me AccelerateXWithRudder clone start(me)
        manage(x)
        self y := me AccelerateYWithElevator clone start(me)
        manage(y)
        self z := me AccelerateZWithThrottle clone start(me)
        manage(z)
        
        loop(
            x target_accel := target_accel at(0)
            y target_accel := target_accel at(1)
            z target_accel := target_accel at(2)
            pass
        )
    )

    AccelerateWCS := coro(me, arg_target_accel,
        if((self ?target_accel) isNil, self target_accel := arg_target_accel)
        
        // forward to local coordinate space
        self lcs := me AccelerateLCS clone start(me)
        manage(lcs)
        
        loop(
            lcs target_accel := me state orient_inv * target_accel
            pass
        )
    )

    SupportAccelWithAileronWCS := coro(me, arg_target_accel,
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
    
    BalancePerceivedAccelWithAileron := coro(me,
        error := me state perceived_accel_lcs at(0)
        loop(
            dt := pass
            new_error := me state perceived_accel_lcs at(0)
            derivative := (new_error - error) / dt
            error := new_error
            me controls setFloat("aileron", (0.15*error + 0.05*derivative) clip(-1,1))
        )
    )
    
    FlyVector := coro(me, arg_target_vector,
        if((self ?target_vector) isNil, self target_vector := arg_target_vector)
        
        self accel := Drone AccelerateWCS clone start(me)
        manage(accel)
        
        self aileron := Drone SupportAccelWithAileronWCS clone start(me)
        manage(aileron)
        
        target_vector ifNil(
            "FlyVector with illegal target vector." println
            self println
        )
        error := target_vector - me state v
        accel target_accel := vector(0,0,0)
        aileron target_accel := vector(0,0,0)
        target_accel := vector(0,0,0)
        
        loop(
            dt := pass
            mod_target_vector := target_vector
            if (mod_target_vector dot(me state front) < 0,
                mod_target_vector = mod_target_vector - me state front scaledBy( mod_target_vector dot(me state front) )
                if (mod_target_vector at(1) < 0, mod_target_vector atSet(1,0))
                mod_target_vector = mod_target_vector scaledBy(target_vector len / mod_target_vector len)
            )
            new_error := mod_target_vector - me state v
            derivative := (new_error - error) scaledBy(1/dt)
            error = new_error
            
            target_accel = error scaledBy(.4) + derivative scaledBy(.15)
            
            accel target_accel = target_accel
            aileron target_accel = target_accel
            #"error: #{error}" interpolate say
        )
    )
    
    MaintainPosition := coro(me, arg_mps_per_m, arg_target_position, arg_target_vector,
        if((self ?mps_per_m) isNil, self mps_per_m := arg_mps_per_m ifNilEval(0.2))
        if((self ?target_position) isNil, self target_position := arg_target_position)
        if((self ?target_vector) isNil, self target_vector := arg_target_vector)
        
        max_dv := 300 / 3.6
        
        fly := Drone FlyVector clone start(me, target_vector)
        manage(fly)
        
        loop(
            delta_v := (target_position - me state p) scaledBy(mps_per_m)
            if (delta_v len > max_dv, delta_v = delta_v scaledBy(max_dv / delta_v len))
            
            fly target_vector := target_vector + delta_v
            pass
        )
    )
    
    FollowSegment := coro(me, arg_a, arg_b, arg_v,
        if((self ?a) isNil, self a := arg_a)
        if((self ?b) isNil, self b := arg_b)
        if((self ?v) isNil, self v := arg_v)
        
        self segment_velocity := vector(0,0,0)
        
        
        mp := Drone MaintainPosition clone start(me)
        manage(mp)
        loop(
            ab_norm := (b-a) normInPlace
            mp target_vector := ab_norm scaledBy(v) + segment_velocity
            mp target_position := a + ab_norm scaledBy(ab_norm dot(me state p - a))
            if (?debug,
                "Target pos: #{mp target_position} err: #{(mp target_position - me state p) len}" interpolate say
            )
            pass
        )
    )
    
    FollowPath := coro(me, navpath, v,
        self fs := Drone FollowSegment clone start(me)
        manage(fs)
        
        fs v := v
        
        while (navpath size > 1,
            segment := navpath currentSegmentSmoothed(me location)
            fs a := segment at(0)
            fs b := segment at(1)
            pass
        )
    )
    
    safety_height := vector(0,3,0)
    descent_angle := 7*Number constants pi / 180
    descent_speed := 160 / 3.6
    final_length := 1200
    
    ReturnToFinal := coro(me, rwy, dangle, final,
        height := final * dangle sin
        a := rwy runwayBegin + Drone safety_height
        b := rwy runwayEnd + Drone safety_height
        d := (b-a) atSet(1,0) norm
        up := vector(0,1,0)
        right := up % d
        p0 := me state p
        p1 := b + d scaledBy(0.3*final) + up scaledBy(height)
        p2 := p1 + right scaledBy(final*0.3)
        p3 := a - d scaledBy(final) + up scaledBy(height) + right scaledBy(final*0.3)
        p4 := a - d scaledBy(final) + up scaledBy(height) + right scaledBy(final*0.1)
        
        self fp := Drone FollowPath clone start(me, NavPath clone with(list(p0,p1,p2,p3,p4)), 350/3.6)
        manage(fp)
        
        while( fp running,
            pass
        )
    )
    
    FinalApproach := coro(me, rwy, dangle,
        fs :=  me FollowSegment clone start(me)
        fs v := me descent_speed
        manage(fs)
        #fs debug:= true
        
        loop (
            a := rwy runwayBegin + Drone safety_height
            b := rwy runwayEnd + Drone safety_height
            p0 := a + (a-b) atSet(1,0) norm scaledBy(dangle cos) + vector(0,dangle sin, 0)
            if( ((me state p) - a) len <= 100, break)
            fs a := p0
            fs b := a
            fs segment_velocity := rwy velocity
            pass
        )
        
        # safety checks
        if ((me state v - rwy velocity) len between(me descent_speed - 8/3.6, me descent_speed + 4/3.6) not,
            "Your speed doesn't look good" say
            return
        )
        if (rwy isRunwayFree not,
            "Runway isn't free" say
            return
        )
        d := p0 - a
        p_proj := a + (me state p - a) projectedOn(d)
        if ((p_proj - me state p) len > 2.5,
            "You are to far away from glide slope" say
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
        
        while( (me state v - rwy velocity) len > 2,
            pass
        )
        sleep(1)
        
        self status := "LANDED"
    )
        
        
    WaitForLandingClearance := coro(me, rwy,
        if (rwy isRunwayLocked not,
            rwy lockRunway(me)
            return
        )
        height := rwy requestHoldingLevel(me)
        "Holding at level #{height}" interpolate println
        
        self fp := Drone FollowPath clone
        manage(fp)
        loop (
            if ( fp running not,
                if (rwy isRunwayLocked not,
                    rwy releaseHoldingLevel(me)
                    rwy lockRunway(me)
                    return
                )
                a := rwy runwayBegin + Drone safety_height
                b := rwy runwayEnd + Drone safety_height
                d := (b-a) atSet(1,0) norm
                up := vector(0,1,0)
                left := (up % d) neg
                
                waypoints := list
                n := 64
                n repeat(i,
                    alpha := 2*Number constants pi * i / n
                    wpt := a + up scaledBy(height) + left scaledBy(2000) + vector(alpha cos, 0, - alpha sin) scaledBy(1000)
                    #wpt println
                    waypoints append( wpt )
                )
                fp start(me, NavPath clone with(waypoints), 350/3.6)
            )
            pass
        )
    )
        
    
    PerformLanding := coro(me, rwy, get_eaten,
        self hold := Drone WaitForLandingClearance clone start(me, rwy)
        manage(hold)
        while (hold running, pass)
        
        # descent angle used to calculate begin of final
        dangle := Drone descent_angle
        final := Drone final_length
        height := final * dangle sin
        a := rwy runwayBegin + Drone safety_height
        b := rwy runwayEnd + Drone safety_height
        d := (b-a) atSet(1,0) norm
        up := vector(0,1,0)
        right := up % d
        
        final_begin := a - d scaledBy(final) + up scaledBy(height)
        
        #"Proceeding to final" say
        self to_final := Drone FollowPath clone start(me,
            NavPath clone with(list(me state p, final_begin - right scaledBy(600), final_begin)),
            250/3.6)
        manage(to_final)
        
        while ((me state p - final_begin) len > 300,
            pass
        )
        to_final interrupt
        
        loop(
            self fa := Drone FinalApproach clone start(me, rwy, dangle)
            manage(fa)
            me controls setBool("landing_gear", true)
            me controls setBool("landing_hook", true)
            
            while (fa running,
                pass
            )
            
            if ((fa ?status) == "LANDED",
                me setControlMode(Actor UNCONTROLLED)
                if (get_eaten,
                    while (rwy droneEatable(me) not,
                        pause(1)
                    )
                    rwy eatDrone(me)
                )
                break
                #we're done!
            )
            
            # else let's play a little safer this time
            final = final + 200
            #"Returning to final" say
            self ret := Drone ReturnToFinal clone start(me, rwy, dangle, final)
            manage(ret)
            me controls setBool("landing_gear", false)
            while (ret running,
                pass
            )
        )
    )
    
    Land := coro(me, rwy, get_eaten,
        p := rwy location
        ex := try(
            (me == Game viewSubject) ifTrue("Travel to runway" say)
            travel := me TravelTo clone start(me, vector(p x, p z))
            while(travel running, pass)
            (me == Game viewSubject) ifTrue("Landing" say)
            manage( me PerformLanding clone start(me, rwy, get_eaten) )
            loop(pass)
        )
        
        if(rwy isRunwayLockedTo(me), rwy unlockRunway)
        
        ex pass
    )
            
                
    TakeOff := coro(me, runway,
        me controls setBool("landing_hook", false)
        me controls setBool("landing_gear", true)
        
        me controls setFloat("aileron", 0)
        me controls setFloat("elevator", -0.2)
        me controls setFloat("rudder", 0)
        me controls setFloat("throttle", 0)
        
        loop(
            obstacles := Game queryActorsInCapsule(
                me location + me state front scaledBy(15),
                me location + me state front scaledBy(200),
                15)
            obstacles remove(me)
            obstacles remove(runway)
            
            if (obstacles isEmpty, break)
            sleep(2)
        )
            
            
        sleep(0.5)
        me controls setFloat("throttle", 1)
        sleep(4)
        me controls setFloat("elevator", 0)
        me controls setBool("landing_gear", false)
    )
    
    FlyInFormation := coro(me, arg_partner, arg_pos,
        if((self ?partner) isNil, self partner := arg_partner)
        if((self ?pos) isNil, self pos := arg_pos)
        
        self mp := Drone MaintainPosition clone start(me, 0.5)
        manage(mp)
        
        while(partner isAlive,
            mp target_vector := partner getMovementVector
            mp target_position := partner getLocation + partner getOrientation * pos
            pass
        )
        
        partner = nil
    )
    
    TravelTo := coro(me, target,
        if((self ?travel_height) isNil, self travel_height := me TRAVEL_HEIGHT)
        if((self ?travel_speed) isNil, self travel_speed := me TRAVEL_SPEED)
        (Game viewSubject == me) ifTrue(("TravelTo speed=" .. self travel_speed * 3.6) say)
        steps_per_frame := 5

        segment_length := 300
        
        p := me location2
        path := list
        path append( vector(p at(0), Terrain heightAt(p x, p y) + travel_height, p at(1) ) )
        
        i := 0
        loop(
            normal := Terrain normalAt(p x, p y)
            to_target := target - p
            dist := to_target length
            d := to_target norm
            
            if ( (p - target) length <= segment_length,
                p = target
                height := Terrain heightAt(p x, p y)
                path append( vector(p at(0), height + travel_height, p at(1) ) )
                break
            ,
                p = p + (d+normal xz) norm scaledBy( segment_length )
                height := Terrain heightAt(p x, p y)
                path append( vector(p at(0), height + travel_height, p at(1) ) )
            )
            
            
            #if (Game viewSubject == me,
            #    "i:#{i} dist:#{(p - target) length asString(2)}" interpolate say
            #)
            
            if (i % steps_per_frame == 0,
                sleep(0)
            )
            i = i+1
        )
        
        navpath := NavPath clone with(path)
        #writeln("Navpath: ", navpath size, " points: ", navpath path)
        manage ( me FollowPath clone start(me, navpath, travel_speed) )
        while( navpath done not,
            pass
        )
    )
    
    TravelTo2 := coro(me, target,
        if((self ?travel_height) isNil, self travel_height := me TRAVEL_HEIGHT)
        self fv := manage( me FlyVector clone start(me) )
        
        slopemax := 35 * Number constants pi / 180
        
        loop(
            p := me location2
            dist := (target - p) length
            if (dist < 500, break)
            d := (target - p) norm
            normal := Terrain normalAt(p x, p y)
            if (normal xz dot(d) < 0,
                tangent := (vector(0,1,0) % normal) xz
                slope := (normal y acos / slopemax)  clip(0,1)
                #if (Game viewSubject == me,
                #    "normal: #{normal} d: #{d} tangent: #{tangent} slope: #{slope}" interpolate say
                #)
                if (d dot(tangent) < 0, tangent negInPlace)
                if (tangent length > 0.00001,
                    tangent := tangent norm
                ,
                    tangent := d clone
                )
                d := d + (tangent-d) scaledBy(slope)
                d := d norm
            )
            d = d scaledBy(me TRAVEL_SPEED)
            fv target_vector := vector( d x, (travel_height - me state height) * 0.1, d y )
            pass
        )
    )
    
    Patrol := coro(me, arg_pos, arg_radius,
        ex := try(
            if((self ?pos) isNil, self pos := arg_pos)
            if((self ?radius) isNil, self radius := arg_radius)
            
            travel := manage ( me TravelTo clone )
            travel travel_height := 1500
            loop(
                if (travel running not,
                    azimuth := 2*Number constants pi * Random value
                    dist := Random value * radius
                    target_pos := pos + vector(azimuth cos, azimuth sin) scaledBy(dist)
                    travel start(me, target_pos, radius/10)
                )
                
                sleep(2 + 3*Random value)
                
                me targeter selectNearestHostileTarget
                me targeter currentTarget ifNonNil(
                    me command_queue prependCommand(
                        Command Attack clone with(me targeter currentTarget))
                )
            )
        )
        
        ex pass
    )
    
    AdHocCommand := method(
        if (hasEffectiveAmmo) then(
            return Command Patrol clone with(self location2, 4000)
        ) elseif ( self hasSlot("started_from") ) then(
            return Command Land clone with(self started_from)
        ) else (
            return Command Patrol clone with(self location2, 4000)
        )
    )
    
    ExecuteCommand := coro(me, command,
        command action switch(
            Command ATTACK, do(
                target := command argActor link
                target ifNonNil(
                    task := manage( me Attack clone start(me, target) )
                    while(task running, pass)
                )
            ),
            Command GOTO, do(
                me callSubordinates
                task := manage( me TravelTo clone start(me, command argVec2) )
                while(task running, pass)
            ),
            Command PATH, do(
                me callSubordinates
                task := manage( me FollowPath clone start(
                    me,
                    command argPath,
                    command argFloat ifNilEval(me TRAVEL_SPEED)
                ) )
                while(task running, pass)
            ),
            Command LAND, do(
                me dispatchSubordinates(command)
                rwy := command argActor link
                get_eaten := command argBool
                if (rwy and rwy isAlive,
                    #"Performing landing." say
                    task := manage( me Land clone start(me, rwy, get_eaten) )
                    while(task running, pass)
                ,
                    "The runway doesn't exist anymore." say
                )
                
            ),
            Command TAKEOFF, do(
                me dispatchSubordinates(command)
                rwy := command argActor link
                if (rwy and rwy isAlive,
                    #"Performing takeoff." say
                    task := manage( me TakeOff clone start(me, rwy) )
                    while(task running, pass)
                ,
                    "The runway doesn't exist anymore." say
                )
                
            ),
            Command PATROL, do(
                me callSubordinates
                task := manage( me Patrol clone start(me, command argVec2, command argFloat) )
                while(task running, pass)
            ),
            Command JOIN, do(
                me callSubordinates
                leader := command argActor link
                if (leader and leader isAlive,
                    p := leader formationPositionOf(me)
                    task := manage( me FlyInFormation clone start(me, leader, p) )
                    while(task running, pass)
                ,
                    "Can't join, leader seems to be dead." say
                )
            ),
            ("Command " .. command action .. " not implemented yet.") say
        )
    )
    
    ai := coro(me,
        #self fly := Drone FollowSegment clone start(me, vector(0,1500,0), vector(0,1500,1), v)
        #manage(fly)
        manage (me ExecuteCommandQueue clone start(me))

        loop(
            pass
        )
    )
    
    on("linked",
        self dispenseDecoys := DispenseDecoys clone start(self)
    )
    on("unlinked",
        dispenseDecoys interrupt
    )
    
    on("start_ai",
        # ai depends on flightState. Io will run coros in LIFO order, so it seems
        # to be ok this way.
        self _ai := ai clone start(self)
        self state := FlightState clone start(self)
        
        ("AI of Drone ".. self uniqueHexId .. " started") println
    )
    on("stop_ai",
        _ai interrupt
        state interrupt
        
        ("AI of Drone ".. self uniqueHexId .. " interrupted") println
    )
)

