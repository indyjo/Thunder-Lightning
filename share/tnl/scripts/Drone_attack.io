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
    
    attackWithMissileSalvo := coro(me, target,
        dirToTarget := block(
            (target location - me location) norm
        )
        
        self fv := me flyVector clone start(me)
        fv target_vector := dirToTarget call * (300 / 3.6)
        fv tag := "attackWithMissileSalvo"
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
    
    gainDistance := coro(me, target,
        toTarget := block(
            target location - me location
        )
        
        
        self fv := me flyVector clone start(me)
        fv tag := "gainDistance"
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
    
    chooseAttackTask := method(target,
        dist := (target location - self location) len
        if (dist > 1000,
            attackWithMissileSalvo clone
        ,
            gainDistance clone
        )
    )
    
    attack := coro(me, target,
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
                attackTask = me chooseAttackTask(target) start(me, target)
            )
        
            ex := try( sleep(0.5) )
            ex catch(InterruptedException,
                attackTask interrupt
                abort = true
            )
        )
    )
)

