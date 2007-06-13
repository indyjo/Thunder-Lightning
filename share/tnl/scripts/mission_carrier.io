addCarrierOld := method(pos, orient,
    carrier := SimpleActor clone
    carrier makeRigid(1,1,1,1,vector(0,0.1,0))

    model := ModelMan query((Config model_dir) .. "/carrier/carrier.obj")
    model setCullmode(Model CULL_NEGATIVE)
    carrier setModel(model)
    
    carrier setLocation(pos)
    carrier setOrientation(orient)

    Game addActor(carrier)
    carrier
)

addCarrier := method(pos, orient,
    carrier := Carrier clone
    
    carrier setLocation(pos)
    carrier setOrientation(orient)

    Game addActor(carrier)
    carrier
)

startup := method(
    if (Game controlledActor,
        self me := Game controlledActor
        mypos := me getLocation
        myorient := me getOrientation
        myvelocity := me getMovementVector
    ,
        mypos := vector(11341,400,-1008)
        myorient := matrix(1, 0, 0
                           0, 1, 0
                           0, 0, 1)
        myvelocity := myorient*vector(0, 0, 90)
    )

    if ((?me) isNil,
        self me := Drone clone
        me setLocation(mypos)
        #me setOrientation(myorient)
        me setMovementVector(myvelocity)
        me setFaction(us)
        Game addActor(me)
    )
    
    corient := matrix( 1, 0, 0
                       0,  1, 0
                       0,  0, 1)
    mypos atSet(1,0,0)
    self carrier := addCarrier(mypos + myvelocity*25, corient)

    Game setControlledActor(me)
    Game setView(me, 0)
    
    me carrier := carrier
    
    me ai := coro(me,
        self land := Drone performLanding clone start(me, me carrier)
        manage(land)

        loop(
            pass
        )
    )
    
    3 repeat(
        drone := me clone
        drone setLocation(mypos + vector(3000*rand2, 800, 3000*rand2))
        drone ai := me ai
        drone setControlMode(Actor AUTOMATIC)
        Game addActor(drone)
    )
)

shutdown := method(
    self removeSlot(me)
)


