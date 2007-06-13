Carrier do(
    init := method(
        self reservedHoldingLevels := list
    )

    runwayBegin := method( getOrientation * vector(-0.081, 13.243, -65.837) + getLocation)
    runwayEnd := method( getOrientation * vector(-19.456, 13.243, 28.497) + getLocation)
    
    runwayLocked := nil
    
    isRunwayFree := method(
        if(runwayLocked isNil not and runwayLocked isAlive not,
            runwayLocked = nil
        )
        runwayLocked isNil
    )
    lockRunway := method(aircraft, runwayLocked = aircraft )
    runwayFreed := method( runwayLocked = nil )
    
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

