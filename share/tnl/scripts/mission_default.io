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

addCarrier := method(pos, angle,
    angle ifNil( angle := 95 )
    alpha := angle * Number constants pi / 180
    orient := matrix(  alpha cos,  0, -alpha sin
                               0,  1,          0
                       alpha sin,  0,  alpha cos)
    
    pos ifNil( pos := vector(17200,0,2000) )

    self carrier := Carrier clone
    
    carrier setLocation(pos)
    carrier setOrientation(orient)
    carrier setControlMode(Actor AUTOMATIC)

    Game addActor(carrier)
    carrier
)

addTank := method(pos,
    tank := Tank clone
    tank setLocation(pos)
    tank setFaction(them)
    tank setControlMode(Actor AUTOMATIC)
    Game addActor(tank)
    tank
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
    self fp := Tank followPath clone start(me, waypoints)
    manage(fp)
    loop(
        pass
    )
)

FollowStupidTank := coro(me,
    self ft := Tank followTank clone start(me, me tank_to_follow, me xpos, me zpos)
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
    
    tankpos := vector(6543,0,1416)
    tankpos2 := tankpos xz
    
    self tank1 := addTank(tankpos)
    tank1 setLocation(tankpos)
    tank1 ai := FollowStupidPath
    addKillObjective(tank1)

    self tank2 := addTank(tankpos + vector(30, 0 , -30) )
    tank2 tank_to_follow := tank1
    tank2 xpos := 20
    tank2 zpos := -35
    tank2 ai := FollowStupidTank
    addKillObjective(tank2)

    self tank3 := addTank(tankpos + vector(-20, 0, -30))
    tank3 tank_to_follow := tank1
    tank3 xpos := -20
    tank3 zpos := -35
    tank3 ai := FollowStupidTank
    addKillObjective(tank3)
    
    self shooting_tanks := list
    2 repeat(
      tank := addTank(vector(6543,0,1386) + randvec scaledBy(1500))
      shooting_tanks append(tank)
    )
    
    self carrier := addCarrier
    carrier setFaction(us)
    addSurviveObjective(carrier)
    
    self wingman1 := carrier spawnDrone
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
    wingman1 setControlMode(Actor AUTOMATIC)
    
    self wingman2 := carrier spawnDrone
    wingman2 command_queue appendCommand(
        Command Takeoff clone with(carrier))
    wingman2 command_queue appendCommand(
        Command Attack clone with(tank3))
    wingman2 command_queue appendCommand(
        Command Attack clone with(tank2))
    wingman2 command_queue appendCommand(
        Command Attack clone with(tank1))
    wingman2 command_queue appendCommand(
        Command Land clone with(carrier))
    wingman2 setControlMode(Actor UNCONTROLLED)
    
    Game setView(wingman2, 0)

    self evil_carrier := addCarrier(vector(-4675,0,18), 0)
    evil_carrier setFaction(them)
    
    self drone1 := evil_carrier spawnDrone
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
    drone1 setControlMode(Actor AUTOMATIC)

    self drone2 := evil_carrier spawnDrone
    drone2 command_queue appendCommand(
        Command Takeoff clone with(evil_carrier))
    drone2 command_queue appendCommand(
        Command Attack clone with(wingman2))
    drone2 command_queue appendCommand(
        Command Attack clone with(wingman1))
    drone2 command_queue appendCommand(
        Command Goto clone with(evil_carrier location2, 2000))
    drone2 command_queue appendCommand(
        Command Land clone with(evil_carrier))
    drone2 setControlMode(Actor AUTOMATIC)
    
    "DefaultMission startup end" println
)

cleanup := method(
    briefing interrupt
    briefing = nil
    
    tank1 = tank2 = tank3 = shooting_tanks = nil
    carrier = nil
    wingman1 = wingman2 = nil
)

