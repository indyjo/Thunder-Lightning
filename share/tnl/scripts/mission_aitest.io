name="AI Demo"
description=(
    "This is a fully choreographed demonstration of Thunder&Lightning's " ..
    "AI capabilities.\n\nThree Lightning aircraft take off from the carrier to " ..
    "perform an air show including some dangerous stunts.")

addCarrier := method(
    alpha := 0 * Number constants pi / 180
    corient := matrix( alpha cos,  0, -alpha sin
                               0,  1,          0
                       alpha sin,  0,  alpha cos)
    cpos := vector(6543,0,1416) scaledBy(1.4)
    
    carrier := Carrier clone
    
    carrier setLocation(cpos)
    carrier setOrientation(corient)
    carrier setFaction(us)
    carrier setControlMode(Actor AUTOMATIC)

    Game addActor(carrier)
    carrier
)

path1 := method(p,
    list(
        p + vector(2000,450,1000),
        p + vector(2000,450,1500),
        p + vector(1500,450,2000),
        p + vector(500,450,2000),
        p + vector(-10,30,1500),
        p + vector(-10,30,500),
        p + vector(-10,30,-1200),
        p + vector(-10,30,-1600),
        p + vector(-10,500,-2100),
        p + vector(-10,1000,-1600),
        p + vector(-10,1000,0),
        p + vector(-10,600,500),
        p + vector(-10,30,0),
        p + vector(-10,30,-700),
        p + vector(-10,30,-2300)
    )
)

path2 := method(p,
    list(
        p + vector(2000,450,1000),
        p + vector(2000,450,1500),
        p + vector(1500,450,2000),
        p + vector(500,450,2000),
        p + vector(10,30,1500),
        p + vector(10,30,500),
        p + vector(10,30,-1200),
        p + vector(10,30,-1600),
        p + vector(10,500,-2100),
        p + vector(10,1000,-1600),
        p + vector(10,1000,0),
        p + vector(10,600,500),
        p + vector(10,30,0),
        p + vector(10,30,-700),
        p + vector(10,30,-2300)
    )
)

path3 := method(p,
    list(
        p + vector(2000,450,1000),
        p + vector(2000,450,1500),
        p + vector(1500,450,2000),
        p + vector(500,450,2000),
        p + vector(0,30,1500),
        p + vector(0,30,500),
        p + vector(0,30,-1200),
        p + vector(0,30,-2500),
        p + vector(0,280,-2750),
        p + vector(0,500,-2500),
        p + vector(0,30,-1500),
        p + vector(0,30,500)
    )
)

addDrone := method(carrier, path,
    me := carrier spawnDrone
    
    me command_queue appendCommand( Command Takeoff clone with(carrier))
    me command_queue appendCommand( Command Path clone with( path ) )
    me command_queue appendCommand( Command Land clone with(carrier))
    me
)

startup := method(
    self carrier := addCarrier
    carrier setControlMode(Actor UNCONTROLLED)
    self drone1 := addDrone(carrier, path1(carrier location))
    drone1 setControlMode(Actor AUTOMATIC)
    self drone2 := addDrone(carrier, path2(carrier location))
    drone2 setControlMode(Actor AUTOMATIC)
    self drone3 := addDrone(carrier, path3(carrier location))
    drone3 setControlMode(Actor AUTOMATIC)
    Game setView(drone3, 0)
)

shutdown := method(
    
)

