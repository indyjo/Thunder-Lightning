name="Debug 3"
description=(
    "Not much to do in this mission: A tank and a carrier.")

addCarrier := method(pos, orient,
    carrier := Carrier clone
    
    carrier setLocation(pos)
    carrier setOrientation(orient)
    carrier setFaction(us)

    Game addActor(carrier)
    carrier
)

addTank := method(pos,
    tank := Tank clone
    tank setLocation(pos)
    tank setFaction(us)
    Game addActor(tank)
    tank
)

startup := method(
    alpha := -15 * Number constants pi / 180
    corient := matrix( alpha cos,  0, -alpha sin
                               0,  1,          0
                       alpha sin,  0,  alpha cos)
    cpos := vector(11341,400,-1008)
    cpos atSet(1,0,0)
    self carrier := addCarrier(cpos, corient)
    
    me := addTank(cpos + vector(0,14,0))

    Game setControlledActor(me)
    Game setView(me, 0)
)

shutdown := method(
)


