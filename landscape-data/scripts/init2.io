path := Config scripts_dir
Importer addSearchPath(path)

doFile(path .. "/delegate.io")
doFile(path .. "/Game.io")
doFile(path .. "/Matrix.io")
doFile(path .. "/Actor.io")
doFile(path .. "/inspect.io")
doFile(path .. "/input.io")
doFile(path .. "/Faction.io")
doFile(path .. "/Completion.io")
doFile(path .. "/Drone.io")

complete := method(str, context,
  try( l:= Completion complete(str, context) ) print
  return l
)

rand2 := method(2*(Number random)-1)
randvec := method(vector(rand2,rand2,rand2))

if(Number hasSlot("pi"), Nil , Number pi := 2* 0 acos)
if(Number hasSlot("e"), Nil , Number e := 1 exp)

buildScene := block(
    factions := List clone
    for(i,0,2,
        factions add(Faction clone)
        factions at(i) setDefaultAttitude(Faction HOSTILE)
        factions at(i) setAttitudeTowards(factions at(i), Faction FRIENDLY)
        factions at(i) setName("Faction" + i)
    )


    ntanks := 5
    for(i,0,ntanks-1,
        tank := Tank clone asActor
        R := Matrix rotation3(vector(0,1,0), 2*i*Number pi/ntanks)
        tank setOrientation(R)
        tank setMovementVector(R*vector(0,0,120))
        tank setLocation(R * vector(0,0,-1000))
        tank setControlMode(Actor AUTOMATIC)
        tank setFaction( factions at(3*i/ntanks) )
        Game addActor(tank)
    )

    ndrones := 9
    for(i,0,ndrones-1,
        drone := Drone clone asActor
        R := Matrix rotation3(vector(0,1,0), 2*i*Number pi/ndrones)
        drone setOrientation(R)
        drone setMovementVector(R*vector(0,0,120))
        drone setLocation(R * vector(0,2000,-5000))
        drone setControlMode(Actor AUTOMATIC)
        drone setFaction( factions at(3*i/ndrones) )
        Game addActor(drone)
    )

    Game setControlledActor(drone)
    Game setView(drone, 0)
    Lobby home := method( Game setControlledActor(drone); Game setView(drone, 0) )
)

debugScene := method(
    factions := List clone
    for(i,0,2,
        factions add(Faction clone)
        factions at(i) setDefaultAttitude(Faction HOSTILE)
        factions at(i) setAttitudeTowards(factions at(i), Faction FRIENDLY)
        factions at(i) setName("Faction" + i)
    )
    ndrones := 5
    for(i,0,ndrones-1,
        drone := Drone clone asActor
        R := Matrix rotation3(vector(0,1,0), 2*i*Number pi/ndrones)
        R print
        drone setOrientation(R)
        drone setMovementVector(R*vector(0,0,120)+5*randvec)
        drone setLocation(R * vector(0,2000,-100)+5*randvec)
        drone setControlMode(Actor AUTOMATIC)
        drone setFaction( factions at(i % 3) )
        Game addActor(drone)
    )

    Game setControlledActor(drone)
    Game setView(drone, 0)
    Lobby home := method( Game setControlledActor(drone); Game setView(drone, 0) )
)

//debugScene
buildScene
