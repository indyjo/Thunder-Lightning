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
    if (?(anyObjective) isNil,
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

startup := method(
    "DefaultMission startup begin" println
    
    briefing = Briefing clone start
    
    objectives = list()
    
    tank := Tank clone
    tank setLocation(vector(6543,0,1416))
    tank setFaction(them)
    tank setControlMode(Actor AUTOMATIC)
    
    tank ai interrupt
    x := vector(6543,1416)
    tank ai := Tank followPath clone start(tank, list(
        x+vector(0,0),
        x+vector(0,-200),
        x+vector(200,400),
        x+vector(400,400),
        x+vector(400,000),
        x+vector(600,-100),
        x+vector(600,-200),
        x+vector(400,-400)))
    Game addActor(tank)
    addKillObjective(tank)

    tank2 := Tank clone
    tank2 setLocation(vector(6573,0,1386))
    tank2 setFaction(them)
    tank2 setControlMode(Actor AUTOMATIC)
    tank2 ai interrupt
    tank2 ai := Tank followTank clone start(tank2, tank, 20, -35)
    Game addActor(tank2)
    addKillObjective(tank2)

    tank3 := Tank clone
    tank3 setLocation(vector(6523,0,1386))
    tank3 setFaction(them)
    tank3 setControlMode(Actor AUTOMATIC)
    tank3 ai interrupt
    tank3 ai := Tank followTank clone start(tank3, tank, -20, -35)
    Game addActor(tank3)
    addKillObjective(tank3)
    
    tanks := list(tank2)
    2 repeat(
      tank4 := Tank clone
      tanks append(tank4)
      tank4 setLocation(vector(6543,0,1386) + 100*randvec)
      tank4 setFaction(them)
      tank4 setControlMode(Actor AUTOMATIC)
      tank4 ai interrupt
      tank4 ai := coro(me, tanks,
        #self fl := Tank flock clone start(me, tanks, 80)
        #manage(fl)
        
        self aaf := Tank aimAtAndFire clone start(me)
        manage(aaf)
        
        loop(pass)
      ) start(tank4, tanks)
      Game addActor(tank4)
      //addKillObjective(tank4)
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

    if (?(me) isNil,
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
    wingman_1 setLocation( me getLocation + 30*me getRightVector + 400*me getFrontVector + 60*me getUpVector)
    wingman_1 setControlMode(Actor AUTOMATIC)
    Game addActor(wingman_1)
    addSurviveObjective(wingman_1)
    
    self wingman_2 := me clone
    wingman_2 setLocation( me getLocation - 30*me getRightVector + 480*me getFrontVector + 80*me getUpVector)
    wingman_2 setControlMode(Actor AUTOMATIC)
    Game addActor(wingman_2)
    addSurviveObjective(wingman_2)

    "DefaultMission startup end" println
)

cleanup := method(
    briefing interrupt
    briefing = nil
)

