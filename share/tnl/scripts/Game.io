Game do (
  appendProto(EventTarget)
  
  delegate(
    addActor,
    removeActor,
    queryActorsInSphere,
    queryActorsInCylinder,
    queryActorsInBox,
    queryActorsInCapsule
    ) to (asActorStage)
    
  mainloopTimings := method(
    timings := list
    i := 0
    while ( debugData int("mainloop_" .. i) isNil not,
        timings append(debugData int("mainloop_" .. i))
        i = i+1
    )
    timings
  )
  
  beginProfileRecording := method(
    self mainloop_timings_accum := mainloopTimings
    self @@_profileRecord
  )
  
  endProfileRecording := method(
    timings := mainloop_timings_accum
    self removeSlot("mainloop_timings_accum")
    timings
  )
  
  _profileRecord := method(
    while( self hasSlot("mainloop_timings_accum"), 
      t := mainloopTimings
      self mainloop_timings_accum mapInPlace(i, v, v + t at(i))
      yield
    )
  )
)
