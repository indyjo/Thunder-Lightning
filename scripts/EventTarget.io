EventTarget := Object clone do(
  ////////////////////////////////////
  // Event-handling subsystem           //
  ////////////////////////////////////
  
  // An event consists of a name and an object with arguments.
  // Events can be sent from the C++ backend and from Io scripts.
  // Events can be received from actors and the Game proto.
  // The user can invent arbitrary events, provided that their name
  // does not collide with predefined events.
  // Events should be used for relatively high-level operations.
  // Event handlers return a value.

  // The following events are predefined:

  // Event name   kill
  // Sent to	Actor
  // Arguments    
  // Description  Called when an actor should remove itself from the game
  //              * All held system resources should be released
  //              * All running coroutines should be stopped
  //              * All references to other actors should be released
  //              Failure to do so could introduce memory leaks,
  //              especially if there are circular dependencies between
  //              actors.
  
  // Event name   applyDamage
  // Sent to	Actor
  // Arguments    damage      the damage suffered
  //              domain      the Collidable domain that was hit (0)
  //              projectile  The projectile that hit the actor (nil)
  //              source      The actor that shot the projectile (nil)
  // Description  Called when a missile, a bullet or some other
  //              projectile hits an actor. Damage is a float value in
  //              some standardized unit. The damage is intentionally left
  //              unspecified until a better solution is found.
  
  // Event name   missileShot
  // Sent to	Actor
  // Arguments    source      the actor that launched the missile
  //              target      the actor that is targeted by the missile (self)
  //              missile     the missile itself
  // Description  Called when a missile was launched toward the receiving actor
  
  // Every message handler has access to "thisHandler" which gives a
  // "MessageHandler" object. The handler can then remove itself from
  // The actor it is registered in.
  MessageHandler := Object clone do (
    name := Nil
    registeredIn := Nil
    action := Nil
    remove := method( registeredIn handlers removeAt(name) )
  )
  
  // handlers stores a map name->handler
  handlers := Map clone
  
  // To specify a message handler, use this method. Example:
  // mytank on("missileShot",
  //   writeln("A missile was shot at ", self)
  //   thisHandler remove
  // )
  on := method(name,
    assert(call argCount == 2)
    // arg 0 should evaluate to a string
    if (self hasSlot("handlers") isNil, self handlers := Map clone)
    action := call argAt(1)
    handlers atPut(name,action)
  )
  
  // Enable or disable message logging by setting logMessages
  logMessages := Nil
  
  // this method is also called by the C++ backend,
  // so its signature must not be changed!
  onMessage := method(name, args,
    handler := messageHandler(name)
    if( handler isNil,
      logMessages and writeln("received message ",name,
        " is unhandled by ", self identify)
      return
    )
    logMessages and writeln(handler registeredIn identify,
      " handles message ", name,
      " received by ", self identify)
    locals := Locals clone
    locals appendProto(args)
    locals self := self
    locals thisHandler := handler

    locals doMessage(handler action)
  )
  
  // Define the older sendMessage name as an alias to onMessage
  sendMessage := getSlot("onMessage")
  
  // Searches through the object ancestry to
  // find a handler for 'name'.
  // It returns
  //  * a new handler object or
  //  * nil if no handler was found.
  messageHandler := method(name,
    obj := self contextWithSlot("handlers")
    while(obj,
      handler := MessageHandler clone
      handler name = name
      handler registeredIn = obj
      handler action = obj handlers at(name)
      
      if (handler action, return handler)
      obj = obj ancestorWithSlot("handlers")
    )
    Nil
  )
)
