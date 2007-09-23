name="Debug 1"
description=(
    "THIS IS A DEBUG MISSION\n\n" ..
    "Originally a test for Io's garbage collector. Nothing to see here, move along.")

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

MissionDebug

