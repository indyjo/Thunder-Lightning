buildScene := block(
    Lobby us := Faction clone do (
		writeln("Context: ",thisContext identify)
        setName("Green")
        setDefaultAttitude(HOSTILE)
        setAttitudeTowards(thisContext, FRIENDLY)
    )
    Lobby them := Faction clone do (
        setName("Red")
        setDefaultAttitude(HOSTILE)
        setAttitudeTowards(thisContext, FRIENDLY)
    )


    tank := Tank clone
    tank setLocation(vector(6543,0,1416))
    tank setFaction(them)
    tank setControlMode(Actor AUTOMATIC)
    Game addActor(tank)
    if (tank armament weapon("Cannon") isTriggered,
      writeln("Cannon triggered!")
      , writeln("Cannon not triggered"))

    tank2 := Tank clone
    tank2 setLocation(vector(6573,0,1386))
    tank2 setFaction(them)
    tank2 setControlMode(Actor AUTOMATIC)
    tank2 ai interrupt
    tank2 ai := Tank followTank clone start(tank2, tank, 20, -35)
    Game addActor(tank2)

    tank3 := Tank clone
    tank3 setLocation(vector(6523,0,1386))
    tank3 setFaction(them)
    tank3 setControlMode(Actor AUTOMATIC)
    tank3 ai interrupt
    tank3 ai := Tank followTank clone start(tank3, tank, -20, -35)
    Game addActor(tank3)
    
    tanks := list()
    0 repeatTimes(
      tank4 := Tank clone
      tanks append(tank4)
      tank4 setLocation(vector(6543,0,1386) + 100*randvec)
      tank4 setFaction(them)
      tank4 setControlMode(Actor AUTOMATIC)
      tank4 ai interrupt
      tank4 ai := Tank flock clone start(tank4, tanks, 80)
      Game addActor(tank4)
    )

    ndrones := 4
    for(i,0,ndrones-1,
        drone := Drone clone
        R := Matrix rotation3(vector(0,1,0), 2*i*Number pi/ndrones)
        drone setOrientation(R)
        drone setMovementVector(R*vector(0,0,120))
        loc := R * vector(0,2000,-10000)
        drone setLocation(loc)
        drone setControlMode(Actor AUTOMATIC)
        drone setFaction( them )
        Game addActor(drone)
    )

    Lobby me := Drone clone
    Game addActor(me)
    me setLocation(vector(11341,1518,-1008))
    me setMovementVector(vector(-98.71, -14.53, -57.98))
    me setOrientation(
      matrix(-0.496, -0.181, -0.849
             -0.101,  0.983, -0.151
              0.862,  0.011, -0.506))
    me setFaction(us)

    Lobby wingman_1 := me clone
    wingman_1 setLocation( me getLocation + 30*me getRightVector + 240*me getFrontVector + 60*me getUpVector)
    wingman_1 setControlMode(Actor AUTOMATIC)
    Game addActor(wingman_1)
    
    Lobby wingman_2 := me clone
    wingman_2 setLocation( me getLocation - 30*me getRightVector + 280*me getFrontVector + 80*me getUpVector)
    wingman_2 setControlMode(Actor AUTOMATIC)
    Game addActor(wingman_2)

    //Lobby me := tank
    Game setControlledActor(me)
    Game setView(me, 0)
    Lobby home := method( Game setControlledActor(me); Game setView(me, 0) )
)

buildScene