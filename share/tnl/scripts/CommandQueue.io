CommandQueue := Object clone do (
    newSlot("emergencyCommand")
    newSlot("adHocCommand")
    newSlot("fallbackCommand")
    
    init := method(
        self commands := list
    )
    
    prependCommand := method(c,
        commands prepend(c)
        self
    )
    
    appendCommand := method(c,
        commands append(c)
        self
    )
    
    clearCommands := method(
        commands removeAll
    )
    
    endCurrentCommand := method(
        emergencyCommand ifNonNil(setEmergencyCommand(nil); return)
        if (commands isNotEmpty,
            commands removeFirst
            return
        )
        adHocCommand ifNonNil(setAdHocCommand(nil); return)
        fallbackCommand ifNonNil(setFallbackCommand(nil); return)
    )
    
    currentCommand := method(subject_actor,
        emergencyCommand ifNonNil(
            if (emergencyCommand workLeft(subject_actor),
                setAdHocCommand(nil)
                return emergencyCommand
            ,
                setEmergencyCommand(nil)
            )
        )
        
        while( commands isNotEmpty,
            c := commands first
            if ( c workLeft(subject_actor),
                setAdHocCommand(nil)
                return c
            ,
                commands removeFirst
            )
        )
        
        adHocCommand ifNonNil(
            if (adHocCommand workLeft(subject_actor),
                return adHocCommand
            ,
                setAdHocCommand(nil)
            )
        )

        fallbackCommand ifNonNil(
            if (fallbackCommand workLeft(subject_actor),
                return fallbackCommand
            ,
                setFallbackCommand(nil)
            )
        )
        
        nil
    )
)

