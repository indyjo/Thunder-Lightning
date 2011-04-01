name="Tutorial 2 - Carrier landing"
description=(
    "In this training mission, you start with your Lightning aircraft in the air, " ..
    "well-positioned for a carrier landing.")

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
    carrier setFaction(us)

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

    alpha := -15 * Number constants pi / 180
    corient := matrix( alpha cos,  0, -alpha sin
                               0,  1,          0
                       alpha sin,  0,  alpha cos)
    cpos := mypos + myvelocity*25
    cpos atSet(1,0,0)
    self carrier := addCarrier(cpos, corient)

    if ((?me) isNil,
        self me := Drone clone
        me setLocation(mypos)
        #me setOrientation(myorient)
        me setMovementVector(myvelocity)
        me setFaction(us)
        me command_queue appendCommand(Command Land clone with(carrier))
        Game addActor(me)
    )
    
    Game setControlledActor(me)
    Game setView(me, 0)
    
    me carrier := carrier
    
    3 repeat(
        drone := me clone
        drone setLocation(mypos + vector(3000*rand2, 800, 3000*rand2))
        drone command_queue appendCommand(Command Land clone with(carrier))
        drone setControlMode(Actor AUTOMATIC)
        Game addActor(drone)
    )
)

shutdown := method(
    self removeSlot(me)
)


