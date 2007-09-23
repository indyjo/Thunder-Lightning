name = "Simple Fight"
description = "You against one drone and one tank. Easy, or is it?"

startup := method(
    mypos := vector(11341,1518,-1008)
    myorient := matrix(-0.496, -0.181, -0.849
                       -0.101,  0.983, -0.151
                        0.862,  0.011, -0.506)
    myvelocity := vector(-98.71, -14.53, -57.98)

    ndrones := 1
    for(i,0,ndrones-1,
        drone := Drone clone
        R := Matrix rotation3(vector(0,1,0), 2*i*(Number pi)/ndrones)
        drone setOrientation(R)
        drone setMovementVector(R*vector(0,0,120))
        loc := mypos + myorient*vector(0,0,3000) + R * vector(0,0,-100)
        drone setLocation(loc)
        drone setControlMode(Actor AUTOMATIC)
        drone setFaction( them )
        Game addActor(drone)
        
        drone armament weapon("Sidewinder") setRoundsLeft(0)
    )
    
    ntanks := 1
    for(i,0,ntanks-1,
        tank := Tank clone
        R := Matrix rotation3(vector(0,1,0), 2*i*(Number pi)/ntanks)
        loc := mypos + myorient*vector(0,0,3000) + R * vector(0,0,-100)
        loc atSet(1,0,0)
        tank setOrientation(R)
        tank setMovementVector(R*vector(0,0,12))
        tank setLocation(loc)
        tank setControlMode(Actor AUTOMATIC)
        tank setFaction( them )
        Game addActor(tank)
    )
    

    self me := Drone clone
    Game addActor(me)
    me setLocation(mypos)
    me setMovementVector(myvelocity)
    me setOrientation(myorient)
    me setFaction(us)

    Game setControlledActor(me)
    Game setView(me, 0)
    self home := method( Game setControlledActor(me); Game setView(me, 0) )
)

