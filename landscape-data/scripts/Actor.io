Actor do(
  ALIVE := 0
  DEAD := 1

  UNCONTROLLED := 0
  MANUAL := 1
  AUTOMATIC := 2

  delegate(getFrontVector, getRightVector, getUpVector) to (asPositionProvider)
  delegate(getLocation, getOrientation) to(asPositionProvider)
  delegate(setLocation, setOrientation) to(asPositionReceiver)
  delegate(getMovementVector) to(asMovementProvider)
  delegate(setMovementVector) to(asMovementReceiver)

  likes := method(actor, getFaction likes(actor getFaction))
  dislikes := method(actor, getFaction dislikes(actor getFaction))
)

