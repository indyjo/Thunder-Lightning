Sequence say := method(color,
    if (color isNil, color := vector(0.6,0.8,1))
    Game infoMessage(self, color)
    nil
)

Intro := coro(mission,
    red := vector(1,0,0)
    yellow := vector(1,1,0)
    
    ex := try (
        mission lockCleanup
        "Welcome to Thunder&Lightning!" say(red)
        sleep(8)
        "To skip this tutorial and go to the real action, press the (I) key." say(red)
        sleep(10)
        "This is Commander Wilson, your flight instructor." say
        sleep(8)
        "We will go through some basics for your first flight." say
        sleep(10)
        "The Lightning is controlled using the mouse and the keyboard." say
        sleep(2)
        "A joystick can be used optionally." say
        sleep(8)
        "Mouse movements control basic flight. You can also use the cursor keys." say
        sleep(8)
        "The throttle can be set from 0% to 100% using keys 1,2,3,...,8,9,0." say
        sleep(8)
        throttle := mission me controls float("throttle") * 100
        "Currently, your throttle is set to #{throttle}%" interpolate say
        "First of all, get us into a safer altitude, please." say
        sleep(5)
        "3000 meters should be enough. " say
        sleep(5)
        "Oh, the altitude is indicated on the right side of the HUD." say
        while (Object,
            8 repeat(
                sleep(1)
                if (mission me getLocation at(1,0) >= 3000,
                    break
                )
            )
            if (mission me getLocation at(1,0) >= 3000,
                break
            )
            
            choose(
                "I am waiting for you to climb up to 3000, kid!",
                "I know you can do it!",
                "Just pull the stick and climb, it's easy!",
                "Make sure the nose is over the horizon!",
                "Are you afraid of heights, kid?"
            ) say
        )
        "Ok, now we're high enough." say
        sleep(8)
        "The small numbers arranged like ladder steps each mark 50m of altitude." say
        sleep(2)
        "The big number to the left of them displays your height over ground." say
        sleep(8)
        "Our current altitude is #{mission me getLocation at(1,0) floor}m above sea level." interpolate say
        sleep(8)
        "In a similiar setup, you find your airspeed indicator on the left side." say
        sleep(8)
        "Our current speed is #{(mission me getMovementVector length * 3.6) floor} km/h." interpolate say
        sleep(8)
        "On the lower edge of the screen, you can see which weapon is selected" say
        "as your primary and secondary weapon." say
        sleep(8)
        "Change the primary weapon on the left with the Backspace key" say
        "and fire it with the left mouse button or Ctrl." say
        sleep(8)
        "Switch the secondary weapon on the right with the Enter key" say
        "and fire it with the right mouse button or Space." say
        sleep(8)
        "Your primary weapon is a 2-barrel Vulcan cannon." say
        sleep(8)
        "Your secondary weapons are Sidewinder and Hydra missiles." say
        sleep(8)
        "The sidewinder is a heat-seeking missile that will find its way" say
        "towards your enemy." say
        sleep(8)
        "The Hydra is an unguided but powerful ground attack rocket." say
        sleep(8)
        "To use the Sidewinders, you must lock on to a target." say
        sleep(8)
        "Use the T key to cycle through all targets. Use R to reverse-cycle." say
        sleep(8)
        "With G you select the target closest to the center of your gunsight." say
        sleep(8)
        "H and F cycle through hostile and friendly targets, respectively." say
        sleep(8)
        "Finally, Z (or Y) selects the nearest target." say
        sleep(8)
        "Are you ready for a challenge? Let's try your combat skills!" say
        sleep(5)
        
        mission him := mission addEnemy
        sleep(3)
        "There's an enemy fighter directly in front of you. Get him!" say
        loop(
            8 repeat(
                sleep(1)
                if (mission him isAlive not,
                    break
                )
            )
            if (mission him isAlive not,
                break
            )
            
            choose(
                "Remember you can select hostile targets with the H key!",
                "Use your Sidewinder missiles!",
                "Get him before he gets you!"
            ) say
        )
        
        "That was it, you got him! Congratulations!" say
        sleep(8)
        "This is the end of the introduction." say
        sleep(4)
        "Now have some fun flying, evading and shooting!" say
        sleep(8)
        "Thunder&Lightning was programmed by Jonas Eschenburg" say(red)
        sleep(2)
        "http://tnlgame.net" say(red)
        sleep(4)
        
        mission survivalWatchdog interrupt

        "Special thanks to Sam Lantinga for SDL, Steve Dekorte for Io and others." say(red)
        
        mission unlockCleanup
    )
    ex catch(InterruptedException,
        interruptReq = nil
        if(mission me isAlive,
            "Introduction was aborted." say(red)
        ,
            "I think he's crashed, Jim!" say(yellow)
            sleep(8)
            "Yeah, seems this novice pilot wasn't so smart after all!" say
            sleep(5)
            "Get me a new one, will you?" say
        )
        mission unlockCleanup
    ) catch(Exception,
        "An exception occurred during intro. See console (F11) for details." say(red)
        ex showStack
    )
)

Intro choose := method(
    n := call argCount
    if (n == 0, return nil)
    r := n * Random value
    return call evalArgAt(r floor)
)

SurvivalWatchdog := coro(actor, target_coro,
    while (actor isAlive,
        sleep(0.5)
    )
    target_coro interrupt
)

addEnemy := method(
    enemy := Drone clone
    Game addActor(enemy)
    // set enemy's location to 1000m to the front, but not below zero
    x := me getLocation + (1000*(me getFrontVector))
    security_altitude := Terrain heightAt(x at(0,0), x at(2,0)) + 500
    if (x at(1,0) < security_altitude,
        x atSet(1,0, security_altitude)
    )
    enemy setLocation(x)
    enemy setMovementVector(me getMovementVector)
    enemy setOrientation( me getOrientation )
    enemy setFaction(them)
    enemy setControlMode(Actor AUTOMATIC)
    
    killObjective := KillObjective clone
    killObjective target := enemy
    
    objectives append(killObjective)
    
    enemy
)

startup := method(
    Game on("toggle-introduction",
        MissionManager introMission intro interrupt    
        MissionManager introMission succeed
        thisHandler remove
    )

    if ((?me) isNil,
        self me := Drone clone
        Game addActor(me)
        me setLocation(vector(11341,1518,-1008))
        me setMovementVector(vector(-98.71, -14.53, -57.98))
        me setOrientation(matrix(
            -0.496, -0.181, -0.849
            -0.101,  0.983, -0.151
            0.862,  0.011, -0.506))
        me setFaction(us)
    )
    
    Game setControlledActor(me)
    Game setView(me, 0)
    
    self intro := Intro clone start(self)

    runningObjective := UndecidedWhileCoroRunningObjective clone
    runningObjective target := self intro
    
    surviveObjective := SurviveObjective clone
    surviveObjective target := me
    
    objectives = list(runningObjective, surviveObjective)

    # Start a survival watchdog to watch me and interrupt intro when dead
    self survivalWatchdog := SurvivalWatchdog clone
    survivalWatchdog start(me, intro)

    "Intro mission started" println
)

cleanup := method(
    survivalWatchdog interrupt

    me := him := nil
    
    objectives := list()
)

