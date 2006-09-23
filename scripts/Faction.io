Faction do(
  FRIENDLY := 0
  NEUTRAL := 1
  HOSTILE := 2

  likes := method(other, getAttitudeTowards(other) == FRIENDLY)
  dislikes := method(other, getAttitudeTowards(other) == HOSTILE)

  setSlot("is", method(other, getName == other getName))
)

