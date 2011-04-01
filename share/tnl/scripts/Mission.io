Mission := Object clone
Mission do(
  CompletionWatchdog := coro(mission,
    loop(
      sleep(1)
      states := mission objectives map(evaluate)
      if (states contains(Objective MISSED)) then(
        mission fail
        break
      ) elseif (states contains(Objective UNDECIDED) not) then(
        mission succeed
        break
      )
    )
  )
  
  UNSTARTED := "UNSTARTED"
  RUNNING := "RUNNING"
  SUCCESS := "SUCCESS"
  FAILURE := "FAILURE"
  ABORTED := "ABORTED"
  
  newSlot("status", UNSTARTED)
  objectives := list()
  lockCount := 0
  
  init := method(
    self completionWatchdog := CompletionWatchdog clone
    self endNotifier := Notifier clone
  )
  
  start := method(
    if (status == UNSTARTED,
      //self println
      self startup
      setStatus(RUNNING)
      completionWatchdog start(self)
    )
    status
  )
  abort :=   method( if(status == RUNNING, endWithStatus(ABORTED)) ; status )
  succeed := method( if(status == RUNNING, endWithStatus(SUCCESS)) ; status )
  fail :=    method( if(status == RUNNING, endWithStatus(FAILURE)) ; status )
  
  isRunning := method(status == RUNNING)
  
  addListener := method(obj, endNotifier addListener(obj))
  removeListener := method(obj, endNotifier removeListener(obj))
  
  lockCleanup := method(
    lockCount = lockCount + 1
  )
  unlockCleanup := method(
    if (lockCount == 1 and isRunning not, ?cleanup)
    lockCount = lockCount -1
  )
  
  endWithStatus := method(s,
    setStatus(s)
    endNotifier notifyListeners(missionEnded(self, s))
    if (lockCount == 0, ?cleanup)
  )
  
  us := Faction clone do (
    setName("Blue")
    setColor(vector(0,0,1))
    setDefaultAttitude(HOSTILE)
    setAttitudeTowards(thisContext, FRIENDLY)
  )

  them := Faction clone do (
    setName("Red")
    setColor(vector(1,0,0))
    setDefaultAttitude(HOSTILE)
    setAttitudeTowards(thisContext, FRIENDLY)
  )
  
  neutrals := Faction clone do (
    setName("Green")
    setColor(vector(0,1,0))
    setDefaultAttitude(NEUTRAL)
    setAttitudeTowards(thisContext, FRIENDLY)
  )
  
  name := "<unnamed mission>"
  description := "This mission does not have a description."
)

