addCarrier := method(pos, orient,
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

startup := method(
    if (Game controlledActor,
        self me := Game controlledActor
        mypos := me getLocation
        myorient := me getOrientation
        myvelocity := me getMovementVector
    ,
        mypos := vector(11341,1518,-1008)
        myorient := matrix(-0.496, -0.181, -0.849
                           -0.101,  0.983, -0.151
                            0.862,  0.011, -0.506)
        myvelocity := vector(-98.71, -14.53, -57.98)
    )

    if ((?me) isNil,
        self me := Drone clone
        me setLocation(mypos)
        me setOrientation(myorient)
        me setFaction(us)
        Game addActor(me)
    )

    addCarrier(mypos + myvelocity*5, myorient)

    mypos atSet(1,0,0)
    addCarrier(mypos, matrix(1,0,0;0,1,0;0,0,1))
    
    Game setControlledActor(me)
    Game setView(me, 0)
)

shutdown := method(
    self removeSlot(me)
)


