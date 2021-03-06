EventTarget := Object clone do(
  ////////////////////////////////////
  // Event-handling subsystem       //
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
    name := nil
    registeredIn := nil
    action := nil
    handlerVars := Object clone
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
    //assert(call argCount == 2)
    // arg 0 should evaluate to a string
    if (self hasLocalSlot("handlers") not, self handlers := Map clone)
    
    handler := MessageHandler clone
    handler name := name asSymbol
    handler action := call argAt(1) clone
    handler registeredIn := self
    handler handlerVars := Object clone
    
    logMessages ifTrue(
      writeln("New handler for ", name, " in context ", self uniqueHexId, ": ")
      handler println
    )
    
    handlers atPut(name, handler)
    
    // return the handlerVars slot so we can set some variables accessible in
    // the handler.
    handler handlerVars
  )
  
  // Enable or disable message logging by setting logMessages
  logMessages := false
  
  onMessage := method(name, args,
    logMessages ifTrue( ("Dispatching message " .. name) println)
    dispatchMessage(name,args,self)
    nil
  )
  
  dispatchMessage := method(name,args,original_context,
    handlers := self getLocalSlot("handlers")
    handlers ifNonNil(

      handler := handlers at(name)
      handler ifNonNil(
        logMessages and writeln(handler registeredIn identify,
          " handles message ", name,
          " received by ", self identify)
        locals := Locals clone
        locals appendProto(args)
        locals appendProto(handler handlerVars)

        locals self := original_context
        locals thisHandler := handler

        locals doMessage(handler action)
      )
    )

  
    self protos foreach(p,
      if (p hasSlot("dispatchMessage"),
        p dispatchMessage(name,args,original_context)
      )
    )
  )
)


