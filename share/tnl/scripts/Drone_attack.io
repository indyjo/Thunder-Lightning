Drone do(
    hasSidewindersLeft := method(
        armament weapon("Sidewinder") roundsLeft > 0
    )
    hasVulcanRoundsLeft := method(
        armament weapon("Vulcan") roundsLeft > 0
        false
    )
    hasAntiAirAmmo := method(
        hasSidewindersLeft or hasVulcanRoundsLeft
    )
    hasAntiGroundAmmo := method(
        hasSidewindersLeft or hasVulcanRoundsLeft
    )
    hasAmmoAgainst := method(target,
        target isAirborneTarget and hasAntiAirAmmo or target isGroundTarget and hasAntiGroundAmmo
    )
    
    AttackWithMissileSalvo := coro(me, target,
        dirToTarget := block(
            (target location - me location) norm
        )
        
        self fv := me FlyVector clone start(me)
        fv target_vector := dirToTarget call * (300 / 3.6)
        fv tag := "AttackWithMissileSalvo"
        manage(fv)
        
        while( me state dir dot( dirToTarget call ) < 0.9,
            fv target_vector := dirToTarget call * (300 / 3.6)
            pass
        )
        
        me armament weapon("Sidewinder") trigger
        sleep(1)
        if (Random value > 0.5, me armament weapon("Sidewinder") trigger)
        
        sleep(2)
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
        dist := (target location - self location) len
        if (dist > 7000) then (
            if (Game viewSubject == self, "Travel to target" say)
            return TravelToTarget clone
        ) elseif (dist > 5000 and target isGroundTarget) then(
            if (Game viewSubject == self, "Approach high" say)
            return ApproachHigh clone
        ) elseif (dist > 1000) then(
            if (Game viewSubject == self, "Missile salvo" say)
            return AttackWithMissileSalvo clone
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

