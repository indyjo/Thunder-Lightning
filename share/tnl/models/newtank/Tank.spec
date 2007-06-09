origmodel TankOrig.obj
model Tank.obj

collisionorig TankOrig-lores.obj
collisionmodel Tank-lores.obj

bounding_radius 6
bounds Tank.bounds
boundsexport bounds.obj
exactness 1

root Body 0 0 0
childof Body Turret 0 2.236532 -0.16312
childof Turret Cannon 0 2.273808 1.65294
childof Turret MachineGun 0 2.336049 -1.867034

domain 1 Turret:* Cannon:* MachineGun:*
domain 2 *:*Chain *:*Bottom

pointof Cannon CannonTip 0 2.273813 5.55294
pointof Cannon CannonTipFront 0 2.273813 6.55294
pointof Cannon CannonTipUp 0 3.273813 5.55294
pointof Cannon CannonCamera 0 2.75 1.75
pointof MachineGun MGTopRight 1.125463 2.473674 -0.798284
pointof MachineGun MGBottomRight 1.125463 2.198424 -0.798284
pointof MachineGun MGTopLeft -1.125463 2.473674 -0.798284
pointof MachineGun MGBottomLeft -1.125463 2.198424 -0.798284
pointof MachineGun MG      0 2.336049 -1.867023
pointof MachineGun MGFront 0 2.336049 -0.867023
pointof MachineGun MGUp    0 3.336049 -1.867023
pointof MachineGun MGCamera 0 3 -2.75

pointof Body Launcher_0 2.441036 1.53235 -2.101367
pointof Body Launcher_1 -2.441036 1.53235 -2.101367
pointof Body Launcher_2 1.954352 1.53401 -2.168714
pointof Body Launcher_3 -1.954352 1.53401 -2.168714

pointof Body Launcher_0_front 2.441036 1.985964 -2.538223
