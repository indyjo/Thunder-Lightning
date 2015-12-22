name="Combat 3 - Carrier Defense"
description=(
    "The carrier is waiting for our drones to return from their mission. " ..
    "It's your job to command the deck gun and protect your ship. " ..
    "Can you survive against an enemy drone attack?\n\n" ..
    "The carrier must remain in place until our drones have returned.")

init := method(
    self anyObjective := AnyObjective clone
    objectives append(anyObjective)
    anyObjective setObjectives(list())
    self actors := list()
)

cleanup := method(
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
    
    carrier setLocation(pos)
    carrier setOrientation(orient)
    carrier setControlMode(Actor AUTOMATIC)

    Game addActor(carrier)
    
    actors append(carrier)
    carrier
)


addAttackingDrone := method(target, carrier, angle, distance,
    alpha := angle * Number constants pi / 180
    ralpha := alpha + Number constants pi
    orient := matrix(  ralpha sin,  0, ralpha cos
                                0,  1,          0
                      -ralpha cos,  0, ralpha sin)
    
    drone := Drone clone
    drone setFaction(them)
    p := target location
    drone command_queue setFallbackCommand(Command Land clone with(carrier))
    drone command_queue appendCommand(Command Goto clone with(vector(p x, p z), 4000))
    drone command_queue appendCommand(Command Attack clone with(target))
    drone setOrientation(orient)
    pos := vector(0, 500, 0) + target location - orient matMult(vector(0,0,distance))
    if (pos y < Terrain heightAt(pos x, pos z) + 500) then(
        pos = vector(pos x, Terrain heightAt(pos x, pos z) + 500, pos z)
    )
    
    drone setLocation(pos)
    drone setControlMode(Actor AUTOMATIC)
    
    Game addActor(drone)
    addKillObjective(drone)
    
    actors append(drone)
    drone
)

addAttackingWave := method(target, carrier, angle, distance, count,
    count repeat( i,
        addAttackingDrone(target, carrier,
            angle + i * 1.5,
            distance + i * 100)
    )
)

addFriendlyDrone := method(carrier, angle, distance,
    alpha := angle * Number constants pi / 180
    ralpha := alpha + Number constants pi
    orient := matrix(  ralpha sin,  0, ralpha cos
                                0,  1,          0
                      -ralpha cos,  0, ralpha sin)
    
    drone := Drone clone
    drone setFaction(us)
    drone started_from := carrier
    drone command_queue appendCommand(Command Patrol clone with(vector(carrier location x, carrier location z), 10000))
    drone setOrientation(orient)
    pos := vector(0, 500, 0) + carrier location - orient matMult(vector(0,0,distance))
    
    drone setLocation(pos)
    drone setControlMode(Actor AUTOMATIC)
    
    Game addActor(drone)
    
    actors append(drone)
    drone
)

addFriendlyWave := method(carrier, angle, distance, count,
    for(i, 0, count,
        addFriendlyDrone(carrier,
            angle + i * 1.5,
            distance + i * 100)
    )
)

addDrone := method(carrier,
    drone := carrier spawnDrone
    drone
)
    
startup := method(
    ///////////////////////////////////////////////////////////////////////////
    // The GOOD people (tm)
    ///////////////////////////////////////////////////////////////////////////
    
    carrier := addCarrier
    carrier setFaction(us)
    addSurviveObjective(carrier)
    Game setView(carrier, 1)
    carrier setControlMode(Actor MANUAL)
    
    "Carrier created" println

    ///////////////////////////////////////////////////////////////////////////
    // The BAD people (tm)
    ///////////////////////////////////////////////////////////////////////////

    bad_carrier := addCarrier(vector(-32000,0,16000), 24)
    bad_carrier setFaction(them)

    addAttackingWave(carrier, bad_carrier, 17, 5000, 2)

    addAttackingWave(carrier, bad_carrier, 53, 13000, 3)

    addAttackingWave(carrier, bad_carrier, 93, 20000, 5)

    addFriendlyWave(carrier, 342, 18000, 5)

    "Enemies created" println
)

