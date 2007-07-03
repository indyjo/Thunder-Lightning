choose := method(
    n := call argCount
    if (n == 0, return nil)
    r := n * Random value
    return call evalArgAt(r floor)
)

addKillObjective := method(target,
    obj := KillObjective clone
    obj setTarget(target)
    objectives append(obj)
)

addSurviveObjective := method(target,
    if ((?anyObjective) isNil,
        self anyObjective := AnyObjective clone
        objectives append(anyObjective)
        anyObjective setObjectives(list())
    )
    obj := SurviveObjective clone
    obj setTarget(target)
    anyObjective objectives append(obj)
)

Briefing := coro(dummy,
    "Commander!" say
    sleep(2)
    "Our spy satellite has spotted tank movements on the island." say
    sleep(6)
    "We have credible intelligence that a group of three tanks has" say
    sleep(2)
    "equipped with new artificial intelligence software." say
    sleep(6)
    "The new AI is not yet completely developed." say
    sleep(6)
    "But we believe it might give the enemy a crucial advantage." say
    sleep(6)
    "All three tanks equipped with the new AI have to be destroyed!" say
    sleep(6)
    "Beware, there are enemy drones and anti-air tanks in the area." say
    sleep(6)
    "You have been assigned a wingman to assist you." say
    sleep(6)
    "Good luck, commander!" say
)

briefing := nil

addCarrier := method(
    alpha := 270 * Number constants pi / 180
    corient := matrix( alpha cos,  0, -alpha sin
                               0,  1,          0
                       alpha sin,  0,  alpha cos)
    cpos := 1.4*vector(6543,0,1416)
    
    carrier := Carrier clone
    
    carrier setLocation(cpos)
    carrier setOrientation(corient)
    carrier setFaction(them)
    carrier setControlMode(Actor AUTOMATIC)

    Game addActor(carrier)
    carrier
)

startup := method(
    "DefaultMission startup begin" println
    
    briefing = Briefing clone start
    
    objectives = list()
    
    tank := Tank clone
    tank setLocation(vector(6543,0,1416))
    tank setFaction(them)
    tank setControlMode(Actor AUTOMATIC)
    
    tank ai := coro(me,
        x := me getLocation
        x := vector(x at(0), x at(2))
        waypoints := list(  x+vector(0,0),
                            x+vector(0,-200),
                            x+vector(200,400),
                            x+vector(400,400),
                            x+vector(400,000),
                            x+vector(600,-100),
                            x+vector(600,-200),
                            x+vector(400,-400))
        self fp := Tank followPath clone start(me, waypoints)
        manage(fp)
        loop(
            pass
        )
    )
    Game addActor(tank)
    addKillObjective(tank)

    tank2 := Tank clone
    tank2 setLocation(vector(6573,0,1386))
    tank2 setFaction(them)
    tank2 setControlMode(Actor AUTOMATIC)
    tank2 tank_to_follow := tank
    tank2 ai := coro(me,
        self ft := Tank followTank clone start(me, me tank_to_follow, 20, -35)
        manage(ft)
        loop(
            pass
        )
    )
    Game addActor(tank2)
    addKillObjective(tank2)

    tank3 := Tank clone
    tank3 setLocation(vector(6523,0,1386))
    tank3 setFaction(them)
    tank3 setControlMode(Actor AUTOMATIC)
    tank3 tank_to_follow := tank
    tank3 ai := coro(me,
        self ft := Tank followTank clone start(me, me tank_to_follow, -20, -35)
        manage(ft)
        loop(
            pass
        )
    )
    Game addActor(tank3)
    addKillObjective(tank3)
    
    tanks := list(tank2)
    2 repeat(
      tank4 := Tank clone
      tanks append(tank4)
      tank4 setLocation(vector(6543,0,1386) + 400*randvec)
      tank4 setFaction(them)
      tank4 setControlMode(Actor AUTOMATIC)
      Game addActor(tank4)
    )

    ndrones := 3
    for(i,0,ndrones-1,
        drone := Drone clone
        R := Matrix rotation3(vector(0,1,0), 2*i*(Number pi)/ndrones)
        drone setOrientation(R)
        drone setMovementVector(R*vector(0,0,120))
        loc := R * vector(0,2000,-8000)
        drone setLocation(loc)
        drone setControlMode(Actor AUTOMATIC)
        drone setFaction( choose(them) )
        Game addActor(drone)
    )

    if ((?me) isNil,
        self me := Drone clone
        Game addActor(me)
        me setLocation(vector(11341,1518,-1008))
        me setMovementVector(vector(-98.71, -14.53, -57.98))
        me setOrientation(
          matrix(-0.496, -0.181, -0.849
                 -0.101,  0.983, -0.151
                  0.862,  0.011, -0.506))
        me setFaction(us)
        Game setControlledActor(me)
        Game setView(me, 0)
    )
    addSurviveObjective(me)

    self wingman_1 := me clone
    wingman_1 setLocation( me getLocation + 30*me getRightVector + 40*me getFrontVector + 5*me getUpVector)
    wingman_1 setControlMode(Actor AUTOMATIC)
    wingman_1 partner := me
    wingman_1 ai := coro(me,
        fly := Drone flyInFormation clone start(me, me partner, vector(20, 5, -40))
        manage(fly)
        loop(pass)
    )
    Game addActor(wingman_1)
    addSurviveObjective(wingman_1)
    
    self wingman_2 := me clone
    wingman_2 setLocation( me getLocation + 60*me getRightVector + 80*me getFrontVector + 10*me getUpVector)
    wingman_2 setControlMode(Actor AUTOMATIC)
    wingman_2 partner := me
    wingman_2 ai := coro(me,
        fly := Drone flyInFormation clone start(me, me partner, vector(40, 10, -80))
        manage(fly)
        loop(pass)
    )
    Game addActor(wingman_2)
    addSurviveObjective(wingman_2)
    
    self carrier := addCarrier

    "DefaultMission startup end" println
)

cleanup := method(
    briefing interrupt
    briefing = nil
)

