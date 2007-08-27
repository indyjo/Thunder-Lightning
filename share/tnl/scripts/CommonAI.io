CommonAI := EventTarget clone do(
    formation_positions := list(vector(50,10,-50), vector(100,20,-100), vector(150,30,-150), vector(200,40,-200))
    formationPosition := method(i, formation_positions at(i))

    addSubordinate := method(subord,
        self hasSlot("subordinates") ifFalse( self subordinates := list )
        subordinates remove(subord)
        subordinates append(subord)
        self
    )
    
    isSubordinate := method(other,
        self hasSlot("subordinates") ifFalse( return false )
        subordinates contains other
    )
    
    formationPositionOf := method(subord,
        idx := subordinates indexOf(subord)
        formationPosition(idx)
    )
    
    dispatchSubordinates := method(command,
        self hasSlot("subordinates") ifFalse (return self)
        subordinates foreach(s,
            s command_queue clearCommands
            s command_queue appendCommand(command)
        )
        self
    )
    
    callSubordinates := method(
        self hasSlot("subordinates") ifFalse (return self)
        subordinates foreach(s,
            s command_queue clearCommands
            s command_queue appendCommand(Command Join clone with(self))
        )
        self
    )
    
    ExecuteCommandQueue := coro(me,
        command := nil
        handler := nil
        
        loop(
            if (handler isNil not and handler running not,
                command = handler = nil
                me command_queue endCurrentCommand
            )
            c := me command_queue currentCommand(me)
            if (c != command,
                if (me == Game viewSubject, ("New command: " .. c ?action) say)
                command = c
                handler ifNonNil(
                    handler interrupt
                )
                
                c ifNonNil(
                    handler = Drone ExecuteCommand clone start(me, command)
                    manage(handler)
                )
            )
            
            c ifNil(
                newCommand := me AdHocCommand
                me command_queue setAdHocCommand(newCommand)
                //("New ad hoc command: " .. newCommand ?action) say
            )
            
            sleep(0.5)
        )
    ) do( tag := "CommonAI" )
    
    
    
    ///////////////////////////////////////////////////////////////////////////
    // Decoy dispension
    ///////////////////////////////////////////////////////////////////////////

    // list of missiles homing into this actor
    missiles := list()
  
    on("missileShot",
        self hasLocalSlot("missiles") ifFalse(self missiles := list())

        missiles append(missile)
        missiles selectInPlace(isAlive)
    )

    on("lockLost",
        self hasLocalSlot("missiles") ifFalse(self missiles := list())

        id := missile actorId
        missiles selectInPlace(m, m isAlive and m actorId != id)
    )
    
    DispenseDecoys := coro(me,
        weapon := me armament weapon("Decoy")
        ex := try( loop(
            mypos := me location
            
            me missiles selectInPlace(isAlive)
            min_dist := me missiles map(missile, (missile location - mypos) length) min
            
            if (min_dist isNil not and min_dist < 5000,
                weapon trigger
            )
            
            sleep(0.8)
        ))
        
        weapon release
        
        ex pass
    )
)

