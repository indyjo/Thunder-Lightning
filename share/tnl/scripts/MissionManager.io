MissionManager := Object clone do(
  path := Config scripts_dir
  currentMission := nil
  
  init := method(
    self missions := list
    list(
      "/mission_intro.io",
      "/mission_carrier.io",
      "/mission_default.io",
      "/mission_simple.io",
      "/mission_aitest.io",
      "/mission_debug.io",
      "/mission_debug2.io"
    ) foreach(name,
      mission := Mission clone
      mission doFile(path .. name)
      missions append(mission)
    )
  )
  
  // The following functions are called from C++. Their interface is fixed.
  numMissions := method(missions size)
  missionName := method(i, missions at(i) name)
  missionDesc := method(i, missions at(i) description)
  runMission  := method(i,
    currentMission = missions at(i) clone
    currentMission start
  )
  // End of interface.
)

MissionManager init

