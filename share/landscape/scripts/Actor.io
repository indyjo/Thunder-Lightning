
Actor do(
  appendProto(EventTarget)
  
  ALIVE := 0
  DEAD := 1

  // The control states an actor can be set to:
  UNCONTROLLED := 0
  MANUAL := 1
  AUTOMATIC := 2

  likes := method(actor, getFaction likes(actor getFaction))
  dislikes := method(actor, getFaction dislikes(actor getFaction))

  )
