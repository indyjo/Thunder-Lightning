MissionManager := Object clone do(
  path := Config scripts_dir
  currentMission := nil
  
  init := method(
    self missions := list
    
    mission_scripts := Directory clone with(path) fileNames select(name,
        name beginsWithSeq("mission_") and name endsWithSeq(".io"))
    mission_scripts foreach(name,
      mission := Mission clone
      mission doFile(path .. "/" .. name)
      missions append(mission)
    )
    missions sortByKey(name)
  )
  
  // The following functions are called from C++. Their interface is fixed.
  numMissions := method(missions size)
  missionName := method(i, missions at(i) name)
  missionDesc := method(i, missions at(i) description)
  runMission  := method(i,
    currentMission = missions at(i) clone
    currentMission addListener(self)
    currentMission start
  )
  // End of interface.
  
  missionEnded := method(mission, status,
    if (status == Mission SUCCESS) then(
      "Congratulations!" say(vector(0,1,0))
      "Mission succesful." say(vector(0,1,0))
    ) elseif (status == Mission FAILURE) then(
      "Mission failed." say(vector(1,0,0))
    )
  )
)

MissionManager init

