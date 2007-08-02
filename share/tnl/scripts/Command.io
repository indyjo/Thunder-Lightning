Command := Object clone do(
    ATTACK := "ATTACK"
    DEFEND := "DEFEND"
    GOTO := "GOTO"
    PATH := "PATH"
    PATROL := "PATROL"
    TAKEOFF := "TAKEOFF"
    LAND := "LAND"
    JOIN := "JOIN"
    
    action   := nil
    argActor := nil
    argBool  := nil
    argVec2  := nil
    argVec3  := nil
    argFloat := nil
    argPath  := nil
    
    abortable := true
    
    workLeft := method(context_actor,
        false
    )
)

Command do(
    Attack := Command clone do(
        action = ATTACK
        
        with := method(target,
            self argActor = WeakLink clone setLink(target)
            self
        )
        
        workLeft := method(subject_actor,
            a := argActor link
            a isNil not and a isAlive
        )
            
    )

    Defend := Command clone do(
        action = DEFEND
        
        with := method(target,
            self argActor = WeakLink clone setLink(target)
            self
        )

        workLeft := method(subject_actor,
            a := argActor link
            a isNil not and a isAlive
        )
    )

    Goto := Command clone do(
        action = GOTO
        
        with := method(position, radius,
            self argVec2 = position
            self argFloat = radius
            self
        )
        
        workLeft := method(subject_actor,
            (subject_actor location2 - argVec2) length >= argFloat
        )
    )
    
    Path := Command clone do(
        action = PATH
        
        with := method(points,
            self argPath = NavPath clone with(points)
            self
        )
        
        workLeft := method(subject_actor,
            argPath done not
        )
    )
    
    Patrol := Command clone do(
        action := PATROL
        
        with := method( position, radius,
            self argVec2 = position
            self argFloat = radius
            self
        )
        
        workLeft := method(subject_actor,
            true
        )
    )
    
    Takeoff := Command clone do(
        action = TAKEOFF
        abortable = false
        
        with := method( runway,
            self argActor = WeakLink clone setLink(runway)
            self
        )

        workLeft := method(subject_actor,
            runway := argActor link
            if( runway isNil, return false)
            
            subject_actor location at(1) < runway runwayEnd at(1) + 100
        )
    )

    Land := Command clone do(
        action = LAND
        
        with := method( runway, opt_getEaten,
            self argActor = WeakLink clone setLink(runway)
            self argBool = opt_getEaten ifNilEval(true)
            self
        )

        workLeft := method(subject_actor,
            runway := argActor link
            if( runway isNil, return false)
            true
        )
    )
    
    Join := Command clone do(
        action = JOIN
        
        with := method( leader,
            self argActor = WeakLink clone setLink(leader)
            self
        )
        
        workLeft := method(subject_actor,
            a := argActor link
            a isNil not and a isAlive
        )
    )
)

