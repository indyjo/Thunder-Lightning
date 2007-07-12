Sequence say := method(color,
    if (color isNil, color := vector(0.6,0.8,1))
    Game infoMessage(self, color)
    nil
)

Intro := coro(mission,
    red := vector(1,0,0)
    yellow := vector(1,1,0)
    
    first_run := true
    
    ex := try (
        mission lockCleanup
        
        if ((mission ?me) isNil or mission me isAlive not,
            mission addMe
            # Start a survival watchdog to watch me and interrupt intro when dead
            mission survivalWatchdog := mission SurvivalWatchdog clone
            mission survivalWatchdog start(mission me, mission intro)
        )

        Game setView(mission carrier, 2)
        
        "Welcome to Thunder&Lightning!" say(red)
        sleep(8)
        Game setView(mission carrier, 0)
        "To skip this tutorial and go to the real action, press the (I) key." say(red)
        sleep(10)
        Game setView(mission carrier, 3)
        "This is Commander James Wilson, your flight instructor." say
        sleep(8)
        "We will go through some basics for your first flight." say
        sleep(10)
        Game setView(mission me, 1)
        "The Lightning is controlled using the mouse and the keyboard." say
        sleep(2)
        "A joystick can be used optionally." say
        sleep(8)
        Game setView(mission me, 0)
        "Mouse movements control basic flight. You can also use the cursor keys." say
        sleep(8)
        "Press the (A) key now to switch to manual control." say
        while(mission me controlMode != Actor MANUAL, 
            pass
        )
        "Good." say
        sleep(5)
        
        "The throttle can be set from 0% to 100% using keys 1,2,3,...,8,9,0." say
        sleep(8)
        throttle := (mission me controls float("throttle") * 100) floor
        "Currently, your throttle is set to #{throttle}%." interpolate say
        if (throttle < 95,
            sleep(2)
            "You must set full throttle to take off from the carrier." say
        )
        sleep(8)
        
        "If you like to, you can press (A) again to switch to autopilot," say
        "in case you feel uncomfortable with the new plane." say
        sleep(8)
        
        loop(
            if ((mission ?me) isNil or mission me isAlive not,
                mission addMe
                Game setView(mission me, 0)
                "Press (A) to switch to manual control." say
            )
            if (mission survivalWatchdog running not,
                mission survivalWatchdog start(mission me, self)
            )
            
            "First of all, get us off this carrier, please." say
            while(mission me location at(1) < 150,
                sleep(1)
            )
            
            if (mission me controlMode == Actor AUTOMATIC,
                "Press (A) again to switch back to manual flight." say
                while (mission me controlMode != Actor MANUAL, pass)
                sleep(1)
                "Ok, plane's in your hands!" say
                sleep(2)
                throttle := (mission me controls float("throttle") * 100) floor
                if (throttle < 40,
                    "Watch out: your throttle is set to #{throttle}%. Press 0 for full throttle." interpolate say
                    sleep(5)
                )
            )
            
            if (mission me controls bool("landing_gear"),
                "Press (L) to retract the landing gear." say
                while (mission me controls bool("landing_gear"), pass)
                sleep(1.0)
                "Landing gear retracted. Good." say
                sleep(5)
            )

            "Now let's go. We should gain some height!" say
            sleep(2)
            "1000 meters should be enough. " say
            loop(
                8 repeat(
                    sleep(1)
                    if (mission me getLocation at(1,0) >= 1000,
                        break
                    )
                )
                if (mission me getLocation at(1,0) >= 1000,
                    break
                )
                
                choose(
                    "I am waiting for you to climb up to 1000, kid!",
                    "I know you can do it!",
                    "Just pull the stick and climb, it's easy!",
                    "Make sure the nose is over the horizon!",
                    "Are you afraid of heights, kid?"
                ) say
            )
            sleep(1)
            "Ok, now we're high enough." say
            sleep(8)
            
            if (first_run,
                "We're going to do your first landing now." say
                sleep(8)
                skip_to_landing := true
                first_run := false
            ,
                skip_to_landing := false
                "Would you like to do another training landing?" say
                sleep(2)
                "Then press the (I) key now!" say
                
                ex := try(sleep(8))
                ex catch(InterruptedException,
                    if( mission me isAlive,
                        skip_to_landing := true
                    ,
                        ex pass
                    )
                ) catch( ex pass )
            )
            
            if (skip_to_landing not,
                combatTraining(mission)
            )
            
            "Get us into a nice landing position please." say
            sleep(8)
            check_dist := true
            check_landing_gear := mission me controls bool("landing_gear") not
            check_landing_hook := mission me controls bool("landing_hook") not
            loop(
                todo := list

                to_carrier := mission carrier runwayBegin - mission me location
                dir_to_carrier := to_carrier norm
                runway_vector := (mission carrier runwayEnd - mission carrier runwayBegin) norm
                if (check_dist,
                    if (to_carrier dot(runway_vector) < 3000,
                        todo append("Start the approach at least 3000m behind the carrier.")
                    ,
                        "You are now far enough behind the carrier. Good." say
                        sleep(5)
                        check_dist := false
                    )
                )

                if (mission me velocity len < 160/3.6,
                    todo append("Don't let your speed drop too much.")
                )
                if (mission me velocity len > 250/3.6,
                    todo append("Reduce your speed to below 250km/h.")
                )
                if (check_landing_gear,
                    if (mission me controls bool("landing_gear") not,
                        todo append("Lower the landing gear (L).")
                    ,
                        "Landing gear successfully lowered." say
                        sleep(5)
                        check_landing_gear := false
                    )
                )
                if (check_landing_hook,
                    if (mission me controls bool("landing_hook") not,
                        todo append("Lower the tail hook (K).")
                    ,
                        "Tail hook successfully lowered." say
                        sleep(5)
                        check_landing_hook := false
                    )
                )
                if (mission me location at(1) > 500,
                    todo append("Descend to below 500m.")
                )
                if (dir_to_carrier dot(runway_vector) < (20 * Number constants pi / 180) cos,
                    todo append("Align yourself with the runway.")
                )
                
                if (todo isEmpty, break)
                chooseFromList(todo) say
                sleep(5)
            )
            mission me command_queue clearCommands
            mission me command_queue appendCommand(
                Command Land clone with(mission carrier, false))
        
            "Good, now reduce the throttle to 11% or 22% (Press 2 or 3)." say
            sleep(5)
            "Try to maintain about 170km/h and land smoothly" say
            sleep(5)
            "Use the brakes (B) to reduce height and brake the aircraft when landed." say
            sleep(5)
            "If you feel insecure, you may press (A) to let the autopilot do the work." say
            
            while(mission carrier droneEatable(mission me) not,
                sleep(1)
            )
            mission survivalWatchdog interrupt
            mission carrier eatDrone(mission me)
            mission me = nil
            "Bravo, well done!" say
            sleep(8)
            "Remember that you can abort the introduction by pressing (I)." say
            sleep(8)
        )
            
        "This is the end of the introduction." say
        sleep(5)
        "Now have some fun flying, evading and shooting!" say
        sleep(8)
        "Thunder&Lightning was programmed by Jonas Eschenburg" say(red)
        sleep(8)
        "Special thanks to Sam Lantinga for SDL, Steve Dekorte for Io and others." say(red)
        sleep(8)
        "Visit http://tnlgame.net" say(red)
        
        mission survivalWatchdog interrupt
        mission unlockCleanup
        mission evaluate := Objective REACHED
    )
    ex catch(InterruptedException,
        interruptReq = nil
        if((mission ?me) isNil not and mission me isAlive,
            "Introduction was aborted." say(red)
            mission evaluate := Objective REACHED
        ,
            ex2 := try(
                "I think he's crashed, Jim!" say(yellow)
                sleep(8)
                "Yeah, seems this novice pilot wasn't so smart after all!" say
                sleep(8)
                "Maybe you're doing something wrong." say(yellow)
                sleep(2)
                "That was the third one in a week!" say(yellow)
                sleep(8)
                "Oh, shut up and get me a new pilot, will you?" say
                sleep(8)
                loop(
                    "Introduction has ended. Press (I) to restart." say(red)
                    sleep(10)
                )
            )
            ex2 catch(InterruptedException,
                mission intro = mission Intro clone start(mission)
                break
            ) catch( pass )
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
Intro chooseFromList := method(l,
    n := l size
    if (n == 0, return nil)
    r := n * Random value
    return l at (r floor)
)


Intro combatTraining := method(mission,
    if (self ?instruments_explained,
        "I already explained the basics so we can proceed to combat training."
        sleep(5)
    )
    if ((self ?instruments_explained) not,
        "Ok, let's do some flight and combat training!" say
        sleep(8)
        "Your altitude is indicated on the right side of the HUD." say
        sleep(8)
        "The small numbers arranged like ladder steps each mark 50m of altitude." say
        sleep(5)
        "The big number to the left of them displays your exact altitude." say
        sleep(8)
        "Currently, you are #{mission me getLocation at(1,0) floor}m above sea level." interpolate say
        sleep(8)
        "In a similiar setup, you find your airspeed indicator on the left side." say
        sleep(8)
        "Our current speed is #{(mission me getMovementVector length * 3.6) floor} km/h." interpolate say
        sleep(8)
        "In the lower left corner of the screen," say
        "you can see which weapon is selected." say
        sleep(8)
        "Change the current weapon with the right mouse button or (Enter)." say
        sleep(8)
        "Press left mouse button or (Ctrl) to fire." say
        sleep(8)
        "The Lightning is equipped with a 2-barrel Vulcan cannon" say
        sleep(8)
        "and there are Sidewinder and Hydra missiles at your command." say
        sleep(8)
        "The sidewinder is a heat-seeking missile that will find its way" say
        "towards your enemy." say
        sleep(8)
        "The Hydra is an unguided but effective ground attack rocket." say
        sleep(8)
        "To use the Sidewinders, you must lock on to a target." say
        sleep(8)
        "With the middle mouse button or (G), you select the target" say
        "closest to the center of your gunsight." say
        sleep(8)
        "Use the (T) key to cycle through all targets. Use (R) to reverse-cycle." say
        sleep(8)
        "H and F cycle through hostile and friendly targets, respectively." say
        sleep(8)
        "Finally, Z (or Y) selects the nearest target." say
        sleep(8)
        "Are you ready for a challenge? Let's try your combat skills!" say
        sleep(5)
        self instruments_explained := true
    )
    
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
    "Now let's get back to the carrier." say
    sleep(8)
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
    x := me getLocation + me getFrontVector scaledBy(1000)
    safety_altitude := Terrain heightAt(x at(0,0), x at(2,0)) + 500
    if (x at(1,0) < safety_altitude,
        x atSet(1,0, safety_altitude)
    )
    enemy setLocation(x)
    enemy setMovementVector(me getMovementVector)
    enemy setOrientation( me getOrientation )
    enemy setFaction(them)
    enemy command_queue appendCommand( Command Attack clone with(me) )
    enemy setControlMode(Actor AUTOMATIC)
    
    enemy
)

addCarrier := method(
    alpha := 285 * Number constants pi / 180
    corient := matrix( alpha cos,  0, -alpha sin
                               0,  1,          0
                       alpha sin,  0,  alpha cos)
    
    cpos := vector(11341,0,-1008)

    self carrier := Carrier clone
    
    carrier setLocation(cpos)
    carrier setOrientation(corient)
    carrier setFaction(us)
    carrier setControlMode(Actor AUTOMATIC)

    Game addActor(carrier)
    carrier
)

addMe := method(
        self me := carrier spawnDrone
        me adHocCommand := nil
        me command_queue appendCommand(Command Takeoff clone with(carrier))
        me controls setFloat("brake", 1)
)

evaluate := Objective UNDECIDED

startup := method(
    Game on("toggle-introduction",
        if (MissionManager introMission isRunning,
            MissionManager introMission intro interrupt
        ,
            thisHandler remove
        )
    )

    addCarrier
    
    self intro := Intro clone start(self)

    objectives = list(self)

    "Intro mission started" println
)

cleanup := method(
    survivalWatchdog interrupt
    survicalWatchdog := nil

    me ifNonNil( me kill )
    (?him) ifNonNil( him kill; him = nil )
    carrier ifNonNil( carrier kill )
    me = carrier = nil
    
    objectives := list()
)

