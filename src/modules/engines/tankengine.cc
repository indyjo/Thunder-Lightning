#include <modules/clock/clock.h>
#include "tankengine.h"
#include <interfaces/ITerrain.h>


#define PI 3.14159265358979323846


TankEngine::TankEngine(Ptr<IGame> game,
    const TankParams & params)
:   RigidEngine(game)
,   TankParams(params)
{
    turret_angle = 0.0f;
    cannon_angle = 0.0f;
}

void TankEngine::setControls(Ptr<DataNode> controls) {
    RigidEngine::setControls(controls);
    tank_controls = new TankControls(controls);
}

void TankEngine::run() {
    RigidEngine::run();
    
    float delta_t = thegame->getClock()->getStepDelta();
    turret_angle += delta_t
            * tank_controls->getTurretSteer()
            * turret_rot_speed;
    if (turret_angle > (2.0f*PI) || turret_angle < 0.0f)
        turret_angle -= floor(turret_angle / (2.0f*PI)) * (2.0f*PI);
    cannon_angle += delta_t
            * tank_controls->getCannonSteer()
            * cannon_rot_speed;
    cannon_angle = std::max(min_cannon_rot,
            std::min(max_cannon_rot, cannon_angle));

    tank_controls->getDataNode()->setFloat("turret_angle", turret_angle);
    tank_controls->getDataNode()->setFloat("cannon_angle", cannon_angle);
    tank_controls->getDataNode()->setVector("rel_cannon_dir", Vector(cos(turret_angle)*sin(cannon_angle),sin(cannon_angle),cos(turret_angle)*cos(cannon_angle)));
}
