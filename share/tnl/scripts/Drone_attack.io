Drone do(
    EstablishLineOfSight := coro(me, target,
        dirToTarget := block(
            (target location - me location) norm
        )
        
        self fv := me FlyVector clone start(me)
        fv target_vector := dirToTarget call * (300 / 3.6)
        fv tag := "Establish LOS"
        manage(fv)
        
        // while we don't have line of sight (LOS)
        while ( (x := Terrain lineIntersection(me location, target location mixedWith(me location, 0.001)) ) isNil not,
            normal := Terrain normalAt(x x, x z)
            to_x := x - me location
            dist := to_x len

            // calculate a vector that
            //  - lies in the plane of to_x and normal
            //  - is perpendicular to to_x
            //  - points in the same direction as normal
            evasion_vect := ((to_x % normal) % to_x) normInPlace
            
            // deviate from course to target by up to 50% (30 degrees)
            fv target_vector := (to_x + evasion_vect scaledBy(dist*0.5)) normInPlace
            //"Correcting course by #{(fv target_vector dot( dirToTarget call ) acos * 180 / Number constants pi) asString(0,1)} degrees" interpolate say
            fv target_vector scaleInPlace( 350 / 3.6 )
            
            sleep(2)
        )
    )
        
    AttackWithSidewinderSalvo := coro(me, target,
        me dispatchSubordinates(me command_queue currentCommand)
        dirToTarget := block(
            (target location - me location) norm
        )
        
        self fv := me FlyVector clone start(me)
        fv target_vector := dirToTarget call * (300 / 3.6)
        fv tag := "Attack with Sidewinder"
        manage(fv)
        
        // now point to target
        while( me state dir dot( dirToTarget call ) < 0.9,
            fv target_vector := dirToTarget call * (400 / 3.6)
            pass
        )
        
        me armament weapon("Sidewinder") trigger
        sleep(1)
        if (Random value > 0.7, me armament weapon("Sidewinder") trigger)
        
        sleep(4)
    )
    
    GainDistance := coro(me, target,
        toTarget := block(
            target location - me location
        )
        
        self fv := me FlyVector clone start(me)
        fv tag := "GainDistance"
        manage(fv)
        
        fv target_vector := (vector(0,1,0) % toTarget call) norm
        if (Random value > 0.5, fv target_vector = fv target_vector * -1)
        
        fv target_vector atSet(1, 0.5)
        fv target_vector = fv target_vector norm * (350 / 3.6)
        sleep(5)
        
        while( toTarget call length < 1500,
            v := toTarget call * -1
            v atSet(1, 0)
            v = v norm * (450 / 3.6)
            fv target_vector := v
            pass
        )
    )
    
    ApproachHigh := coro(me, target,
        target_dist   := 5000
        target_height := 1500
        
        to_target := target location2 - me location2
        d := to_target norm
        target_point_2d := target location2 - d scaledBy(target_dist)
        target_point_3d := vector(  target_point_2d at(0),
                                    target location at(1) + target_height,
                                    target_point_2d at(1))

        self fs := manage( me FollowSegment clone start(me, me location, target_point_3d, 450/3.6) )
        
        loop(
            to_target := target location2 - me location2
            dist := to_target length
            if (dist <= target_dist, break)
            sleep(5)
        )
    )
    
    TravelToTarget := coro(me, target,
        me callSubordinates
        target_dist := 7000
        saved_location := target location2
        
        self tr := manage( me TravelTo clone start(me, target location2) )
        loop(
            sleep(15 + 10*Random value)
            
            to_target := target location2 - me location2
            dist := to_target length
            if (dist <= target_dist, break)
            
            if ((target location2 - saved_location) length > 1000,
                tr interrupt
                tr = manage( me TravelTo clone start(me, target location2) )
                saved_location = target location2
            )
        )
    )
    
    ChooseAttackTask := method(target,
        dist := (target location2 - self location2) len
        if (dist > 7000) then (
            if (Game viewSubject == self, "Travel to target" say)
            return TravelToTarget clone
        ) elseif (dist > 5000 and target isGroundTarget) then(
            if (Game viewSubject == self, "Approach high" say)
            return ApproachHigh clone
        ) elseif (dist > 1000) then(
            if (self hasLineOfSightTo(target),
                if (Game viewSubject == self, "Sidewinder salvo" say)
                return AttackWithSidewinderSalvo clone
            ,
                if (Game viewSubject == self, "Establish line-of-sight" say)
                return EstablishLineOfSight clone
            )
        ) else (
            if (Game viewSubject == self, "Gain distance" say)
            return GainDistance clone
        )
    )
    
    Attack := coro(me, target,
        self state := "ATTACKING"
        me targeter setCurrentTarget(target)
        self attackTask := nil
        self abort := false
        loop(
            target isAlive ifFalse(
                state = "TARGETDESTROYED"
                break
            )
            me targeter currentTarget ifNil(
                state = "LOSTTARGET"
                break
            )
            me hasAmmoAgainst(target) ifFalse(
                state = "OUTOFAMMO"
                break
            )
            abort ifTrue(
                state = "ABORTED"
                break
            )
            
            if (attackTask isNil or attackTask running not,
                attackTask = me ChooseAttackTask(target) start(me, target)
            )
        
            ex := try( sleep(0.5) )
            ex catch(InterruptedException,
                attackTask interrupt
                abort = true
            )
        )
    )
)

