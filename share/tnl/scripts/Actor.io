
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

  location := getSlot("getLocation")
  orientation := getSlot("getOrientation")
  velocity := getSlot("getMovementVector")

  location2 := method( vector(location at(0,0), location at(2,0)) )
  velocity2 := method( vector(velocity at(0,0), velocity at(2,0)) )
  front2 := method( f := getFrontVector; vector(f at(0,0), f at(2,0)) norm )
  right2 := method( f := front2; vector(f at(1,0), -f at(0,0)) )
  
)
