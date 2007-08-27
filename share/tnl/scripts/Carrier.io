Carrier do(
    appendProto(Runway)
    isGroundTarget := true
    
    init := method(
        
        self turret := TurretAI clone do(
            pivot := vector(22.908, 29.255, -18.621)
            turret_control_name := "main_turret_steer_x"
            turret_state_name := "main_turret_angle_x"
            cannon_control_name := "main_turret_steer_y"
            cannon_state_name := "main_turret_angle_y"
        )
        turret weapon := armament weapon("Vulcan")
        
        self reservedHoldingLevels := list
        self runwayLocked := nil
    )
    
    droneSpawnPos := method(n,
        self location + self getUpVector * 15.5 - self getFrontVector * 20 * n
    )
    spawnDrone := method(
        n := 0
        loop(
            x := droneSpawnPos(n)
            if (Game queryActorsInSphere(x, 5) isEmpty,
                break
            )
            n = n+1
        )
        
        drone := Drone clone
        
        drone setLocation( x )
        drone setOrientation( self orientation )
        drone controls setBool("landing_gear", true)
        drone setControlMode(Actor UNCONTROLLED)
        drone setFaction( self getFaction )
        
        Game addActor(drone)
        drone
    )
    eatDrone := method(drone,
        if (droneEatable(drone),
            if (Game viewSubject == drone,
                Game setView(self, 0)
                Game setControlledActor(self)
                drone setControlMode(Actor UNCONTROLLED)
            )
            
            coro(drone,
                sleep(2)
                drone kill
            ) start(drone)
        )
    )
    droneEatable := method(drone,
        l := (runwayEnd - runwayBegin) len
        d := (runwayEnd - runwayBegin) norm
        t := (drone location - runwayBegin) dot(d)
        
        if (t < 0 or t > l,
            return false
        )
        x := runwayBegin + d*t
        if ((x - drone location) len > 7,
            return false
        )
        
        if ((drone velocity - self velocity) length > 2,
            return false
        )
        true
    )

    spawnTank := method(
        tank := Tank clone
        
        tank setLocation( self location - 52* self getFrontVector )
        tank setOrientation( self orientation * Matrix rotation3(vector(0,1,0), Number constants pi))
        tank setControlMode(Actor UNCONTROLLED)
        tank setFaction( self getFaction )
        
        Game addActor(tank)
        tank
    )
    
    runwayBegin := method( getOrientation * vector(-0.081, 13.243, -65.837) + getLocation)
    runwayEnd := method( getOrientation * vector(-19.456, 13.243, 28.497) + getLocation)
    
    ai := coro(me,
        self turret_ai := me turret AttackCloseTargets clone start(me, me turret)
        manage(turret_ai)
        loop(pass)
    )

    on("start_ai",
        self _ai := ai clone start(self)
        ("AI of Carrier ".. self uniqueHexId .. " started") println
    )
    on("stop_ai",
        _ai interrupt
        ("AI of Carrier ".. self uniqueHexId .. " interrupted") println
    )

)

