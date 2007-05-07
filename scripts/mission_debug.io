MissionDebug := Object clone
MissionDebug init := method(
    "MissionDebug init begin" println
    //Drone willFree := method("Drone will free" println)
    2 repeat (
        drone := Drone clone
        "Created drone at #{drone uniqueHexId}" interpolate println
    )
    
    "MissionDebug init end" println
)

Game mission := MissionDebug clone
