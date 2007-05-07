DefaultMission := Object clone

DefaultMission init := method(
    "DefaultMissiion init begin" println
    self us := Faction clone do (
        setName("Blue")
        setDefaultAttitude(HOSTILE)
        setAttitudeTowards(thisContext, FRIENDLY)
    )
    self them := Faction clone do (
        setName("Red")
        setDefaultAttitude(HOSTILE)
        setAttitudeTowards(thisContext, FRIENDLY)
    )

    tank := Tank clone
    tank setLocation(vector(6543,0,1416))
    tank setFaction(them)
    tank setControlMode(Actor AUTOMATIC)
    
    tank ai interrupt
    tank ai := coro(me,
      x := vector(6543,1416)
      self fp:= Tank followPath clone start(me, list(
        x+vector(0,0),
        x+vector(0,-200),
        x+vector(200,400),
        x+vector(400,400),
        x+vector(400,000),
        x+vector(600,-100),
        x+vector(600,-200),
        x+vector(400,-400)))
      manage(fp)
    )
    tank ai start(tank)

    Game addActor(tank)


    ndrones := 4
    for(i,0,ndrones-1,
        drone := Drone clone
        R := Matrix rotation3(vector(0,1,0), 2*i*(Number pi)/ndrones)
        drone setOrientation(R)
        drone setMovementVector(R*vector(0,0,120))
        loc := R * vector(0,2000,-10000)
        drone setLocation(loc)
        drone setControlMode(Actor AUTOMATIC)
        drone setFaction( them )
        Game addActor(drone)
    )

    self me := Drone clone
    Game addActor(me)
    me setLocation(vector(11341,1518,-1008))
    me setMovementVector(vector(-98.71, -14.53, -57.98))
    me setOrientation(
      matrix(-0.496, -0.181, -0.849
             -0.101,  0.983, -0.151
              0.862,  0.011, -0.506))
    me setFaction(us)

    wingman_1 := me clone
    wingman_1 setLocation( me getLocation + 30*me getRightVector + 240*me getFrontVector + 60*me getUpVector)
    wingman_1 setControlMode(Actor AUTOMATIC)
    Game addActor(wingman_1)
    
    wingman_2 := me clone
    wingman_2 setLocation( me getLocation - 30*me getRightVector + 280*me getFrontVector + 80*me getUpVector)
    wingman_2 setControlMode(Actor AUTOMATIC)
    Game addActor(wingman_2)

    //self me := tank
    Game setControlledActor(me)
    Game setView(me, 0)
    "DefaultMission init end" println
)

Game mission := DefaultMission clone

