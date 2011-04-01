Objective := Object clone do (
  REACHED := "REACHED"
  MISSED := "MISSED"
  UNDECIDED := "UNDECIDED"
)

KillObjective := Objective clone do (
  newSlot("target")
  evaluate := method(
    if (target isAlive, UNDECIDED, REACHED)
  )
)

SurviveObjective := Objective clone do (
  newSlot("target")
  evaluate := method(
    if (target isAlive, REACHED, MISSED)
  )
)

UndecidedWhileCoroRunningObjective := Objective clone do(
  newSlot("target")
  evaluate := method(
    if(target running, UNDECIDED, REACHED)
  )
)

AnyObjective := Objective clone do(
  newSlot("objectives", list())
  evaluate := method(
    all_missed := true
    objectives foreach(o,
      o evaluate switch(
        REACHED, return REACHED,
        UNDECIDED, all_missed=false; break,
        nil
      )
    )
    
    if (all_missed, MISSED, UNDECIDED)
  )
)

AllObjective := Objective clone do(
  newSlot("objectives", list())
  evaluate := method(
    all_reached := true
    objectives foreach(o,
      o evaluate switch(
        MISSED, return MISSED,
        UNDECIDED, all_reached=false; break,
        nil
      )
    )
    
    if (all_reached, REACHED, UNDECIDED)
  )
)

