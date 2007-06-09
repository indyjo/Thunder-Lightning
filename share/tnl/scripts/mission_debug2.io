startup := method(
    Tank init := method()
    x := vector(7790,0,1566)
    tank1 := Tank clone
    tank1 setLocation(x)
    tank1 setOrientation(matrix(1,0,0
                                0,1,0
                                0,0,1))
    tank1 ai interrupt
    tank1 controls setFloat("turret_steer", 0.2)
    Game addActor(tank1)

    tank2 := tank1 clone
    tank2 setLocation(x + vector(0,0,-40))
    tank2 ai interrupt
    Game addActor(tank2)
    
    coro(tank,
        sleep(5)
        "Fire!"say
        tank armament weapon("Cannon") trigger
        Clock pause) start(tank2)
    Game setView(tank2, 0)
)

