Stats := Object clone do (
    lastDamagers := Map clone
    kills := List clone
    
    registerDamage := method(damagingActor, damagedActor, damageValue,
        lastDamagers atPut(damagedActor uniqueId asString, damagingActor getFaction getName)
    )
    
    registerKill := method( victim,
        lastDamager := lastDamagers at( victim uniqueId asString)
        killentry := Object clone
        if (lastDamager,
            killentry culprit := lastDamager
            killentry victim := ((victim getFaction) getName)
        ,
            killentry culprit := "<suicide>"
            killentry victim := ((victim getFaction) getName)
        )
        kills add(killentry)
    )
    
    killsBy := method( faction,
        kills select(i,v, v culprit == faction) count
    )

    killsOf := method( faction,
        kills select(i,v, v victim == faction) count
    )

    killsOfBy := method( culprit, victim,
        kills select(i,v, v culprit == culprit and v victim == victim) count
    )
    
    scoreOf := method( faction, killsBy(faction) - killsOf(faction))
    
    allFactions := method(
        factions := Map clone
        kills foreach(i,v, factions atPut(v culprit, Nil); factions atPut(v victim, Nil))
        factions keys
    )
    
    print := method(
        factions := allFactions
        factions sortBy (method(a,b, scoreOf(a) > scoreOf(b)))
        writeln("-----------------------------------")
        writeln("         Faction score kills deaths")
        factions foreach(i, faction,
            writeln(faction rightJustified(16),
                scoreOf(faction) asString rightJustified(6), 
                killsBy(faction) asString rightJustified(6),
                killsOf(faction) asString rightJustified(7))
        )
        
        writeln("-----------------------------------")
    )
)

stats := Stats
Drone do(

    delegate(asPositionProvider, asMovementProvider) to (asActor)
    delegate(asPositionReceiver, asMovementReceiver) to (asActor)
    delegate(getFaction, getNumViews) to (asActor)
    delegate(getFrontVector, getRightVector, getUpVector) to (asPositionProvider)
    delegate(getLocation, getOrientation) to(asPositionProvider)
    delegate(setLocation, setOrientation) to(asPositionReceiver)
    delegate(getMovementVector) to(asMovementProvider)
    delegate(setMovementVector) to(asMovementReceiver)

    onKill := method(
        ex := try(
        stats registerKill(self)
        )
        if (ex,
            ex print
            ex backTraceString print
        )
    )
    
    onDamage := method(damage, domain, projectile, source,
        ex := try(
        if (source isNil, return)
        stats registerDamage(source, self, damage)
        )
        if (ex, ex print)
    )
)
