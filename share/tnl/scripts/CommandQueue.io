CommandQueue := Object clone do (
    newSlot("emergencyCommand")
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
    
    currentCommand := method(subject_actor,
        emergencyCommand ifNonNil(
            if (emergencyCommand workLeft(subject_actor),
                return emergencyCommand
            ,
                setEmergencyCommand(nil)
            )
        )
        
        while( commands isNotEmpty,
            c := commands first
            if ( c workLeft(subject_actor),
                return c
            ,
                commands removeFirst
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

