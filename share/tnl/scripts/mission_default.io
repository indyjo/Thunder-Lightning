name="Triple Tank Takeout"
description=(
    "Our spy satellite has spotted tank movements on the island. " ..
    "We have credible intelligence that a group of three tanks has " ..
    "been equipped with new artificial intelligence software.\n\n" ..
    "Find and destroy the tanks! " ..
    "Your wingman knows where to find them. But beware, " ..
    "the enemy carrier is close and the tanks will be heavily defended.\n\n" ..
    "Good luck, Commander!")

init := method(
    self briefing := Briefing clone
    self actors := List clone

    self anyObjective := AnyObjective clone
    objectives append(anyObjective)
    anyObjective setObjectives(list())
)

cleanup := method(
    briefing interrupt
    briefing = nil
)

addKillObjective := method(target,
    obj := KillObjective clone
    obj setTarget(target)
    objectives append(obj)
)

addSurviveObjective := method(target,
    obj := SurviveObjective clone
    obj setTarget(target)
    anyObjective objectives append(obj)
)

addCarrier := method(pos, angle,
    angle ifNil( angle := 95 )
    alpha := angle * Number constants pi / 180
    orient := matrix(  alpha cos,  0, -alpha sin
                               0,  1,          0
                       alpha sin,  0,  alpha cos)
    
    pos ifNil( pos := vector(17200,0,2000) )

    carrier := Carrier clone
    actors append(carrier)
    
    carrier setLocation(pos)
    carrier setOrientation(orient)
    carrier setControlMode(Actor AUTOMATIC)

    Game addActor(carrier)
    carrier
)

addTank := method(pos,
    tank := Tank clone
    actors append(tank)
    
    tank setLocation(pos)
    tank setFaction(them)
    tank setControlMode(Actor UNCONTROLLED)
    Game addActor(tank)
    tank
)

addGuardingTank := method(pos,
    tank := addTank(pos)
    tank setControlMode(Actor AUTOMATIC)
    tank
)

addDrone := method(carrier,
    drone := carrier spawnDrone
    actors append(drone)
    drone
)
    

FollowStupidPath := coro(me,
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
    self fp := Tank FollowPath clone
    manage(fp)
    loop(
        if (fp running not, fp start(me, NavPath clone with(waypoints)))
        pass
    )
)

FollowStupidTank := coro(me,
    self ft := Tank FollowTank clone start(me, me tank_to_follow, me xpos, me zpos)
    manage(ft)
    loop(
        pass
    )
)

Briefing := coro(dummy,
    sleep(5)
    "Commander!" say
    sleep(2)
    "Our spy satellite has spotted tank movements on the island." say
    sleep(6)
    "We have credible intelligence that a group of three tanks has" say
    sleep(2)
    "been equipped with new artificial intelligence software." say
    sleep(6)
    "Find and destroy the tanks!" say
    sleep(6)
    "Your wingman knows where to find them. But beware," say
    sleep(4)
    "the enemy carrier is close and the tanks will be heavily defended." say
    sleep(4)
    "Good luck, commander!" say
)

briefing := nil

startup := method(
    "DefaultMission startup begin" println
    
    briefing start
    
    objectives = list()
    
    ///////////////////////////////////////////////////////////////////////////
    // The EVIL tanks (tm)
    ///////////////////////////////////////////////////////////////////////////

    tankpos := vector(-4977,0,3476)
    tankpos2 := tankpos xz
    
    tank1 := addTank(tankpos)
    tank1 setLocation(tankpos)
    tank1 ai := FollowStupidPath
    tank1 controlMode asString println
    tank1 setControlMode(Actor AUTOMATIC)
    addKillObjective(tank1)

    tank2 := addTank(tankpos + vector(30, 0 , -30) )
    tank2 tank_to_follow := tank1
    tank2 xpos := 20
    tank2 zpos := -35
    tank2 ai := FollowStupidTank
    tank2 setControlMode(Actor AUTOMATIC)
    addKillObjective(tank2)

    tank3 := addTank(tankpos + vector(-20, 0, -30))
    tank3 tank_to_follow := tank1
    tank3 xpos := -20
    tank3 zpos := -35
    tank3 ai := FollowStupidTank
    tank3 setControlMode(Actor AUTOMATIC)
    addKillObjective(tank3)
    
    addGuardingTank(tankpos + vector(800, 0, -500))
    addGuardingTank(tankpos + vector(-200, 0, 500))
    
    addGuardingTank(vector(567.4, 0, 564.8))
    addGuardingTank(vector(-568.1, 0, -564.5))
    addGuardingTank(vector(-5054.6, 0, -283.48))
    addGuardingTank(vector(836.7, 0, -3904.6))
    
    ///////////////////////////////////////////////////////////////////////////
    // The GOOD people (tm)
    ///////////////////////////////////////////////////////////////////////////
    
    carrier := addCarrier
    carrier setFaction(us)
    addSurviveObjective(carrier)
    
    wingman1 := addDrone(carrier)
    wingman1 command_queue appendCommand(
        Command Takeoff clone with(carrier))
    wingman1 command_queue appendCommand(
        Command Attack clone with(tank1))
    wingman1 command_queue appendCommand(
        Command Attack clone with(tank2))
    wingman1 command_queue appendCommand(
        Command Attack clone with(tank3))
    wingman1 command_queue appendCommand(
        Command Land clone with(carrier))
    
    wingman2 := addDrone(carrier)
    wingman2 command_queue appendCommand(
        Command Join clone with(wingman1))
    wingman2 setControlMode(Actor UNCONTROLLED)
    
    wingman1 addSubordinate(wingman2)
    wingman1 setControlMode(Actor AUTOMATIC)
    
    ///////////////////////////////////////////////////////////////////////////
    // The BAD people (tm)
    ///////////////////////////////////////////////////////////////////////////

    Game setView(wingman2, 0)

    evil_carrier := addCarrier(vector(-5540,0,1128), 0)
    evil_carrier setFaction(them)
    
    drone1 := addDrone(evil_carrier)
    drone1 command_queue appendCommand(
        Command Takeoff clone with(evil_carrier))
    drone1 command_queue appendCommand(
        Command Attack clone with(wingman1))
    drone1 command_queue appendCommand(
        Command Attack clone with(wingman2))
    drone1 command_queue appendCommand(
        Command Goto clone with(evil_carrier location2, 2000))
    drone1 command_queue appendCommand(
        Command Land clone with(evil_carrier))

    drone2 := addDrone(evil_carrier)
    drone2 command_queue appendCommand(
        Command Join clone with(drone1))
    drone2 setControlMode(Actor AUTOMATIC)
    
    drone1 addSubordinate(drone2)
    drone1 setControlMode(Actor AUTOMATIC)
    
    
    wingman2 command_queue clearCommands
    wingman2 command_queue appendCommand(Command Attack clone with(drone1))
    wingman2 command_queue appendCommand(Command Attack clone with(drone2))
    
    "DefaultMission startup end" println
)

