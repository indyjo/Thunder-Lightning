Runway := Object clone do(
    isRunwayFree := method(
        actors := Game queryActorsInCapsule(runwayBegin, runwayEnd, 8)
        actors foreach(actor,
            if (actor type == "Drone",
                return false
            )
        )
        true
    )

    isRunwayLocked := method(
        if(runwayLocked isNil not and runwayLocked isAlive not,
            runwayLocked = nil
        )
        runwayLocked isNil not
    )
    isRunwayLockedTo := method(actor,
        runwayLocked == actor
    )
    lockRunway := method(aircraft, runwayLocked = aircraft )
    unlockRunway := method( runwayLocked = nil )
    
    requestHoldingLevel := method(aircraft,
        level := 0
        loop(
            if (isHoldingLevelFree(level),
                reservedHoldingLevels append( list(aircraft, level) )
                return 500 + 100*level
            )
            level = level +1
        )
    )
    releaseHoldingLevel := method(aircraft,
        reservedHoldingLevels foreach(i,v,
            if (v at(0) == aircraft,
                reservedHoldingLevels removeAt(i)
                break
            )
        )
        nil
    )
    isHoldingLevelFree := method(n,
        reservedHoldingLevels foreach(i,v,
            if (v at(1) == n,
                return false
            )
        )
        return true
    )
)

