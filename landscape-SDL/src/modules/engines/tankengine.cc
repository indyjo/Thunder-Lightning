#include <modules/clock/clock.h>
#include "tankengine.h"
#include <interfaces/ITerrain.h>


#define PI 3.14159265358979323846


TankEngine::TankEngine(Ptr<IGame> game, Ptr<TankControls> ctrls,
    const CarParams & cparams,
    const TankParams & params)
:   CarEngine(game, ctrls, cparams),
    TankParams(params),
    tank_controls(ctrls)
{
    turret_angle = 0.0f;
    cannon_angle = 0.0f;
    setAmmo(0, 6400);
    cannon_state = CANNON_READY;
    ammo_type = 0;
}


void TankEngine::run() {
    CarEngine::run();
    
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
    
    if (isCannonReady() && 
            tank_controls->getAmmoType() != getAmmoType() &&
            getAmmo(tank_controls->getAmmoType()) > 0) {
        setAmmoType(tank_controls->getAmmoType());
    }
    
    if (tank_controls->getReload()) {
        reloadCannon();
        tank_controls->setReload(false);
    }
    
    if (isCannonLoading()) {
        if (reload_time_left <= 0.0f) {
            cannon_state = CANNON_READY;
        }
        reload_time_left -= delta_t;
    }
    
    //ls_warning("fire: %s\n", tank_controls->getFire()?"true":"false");
    //ls_warning("cannon ready: %s\n", isCannonReady()?"true":"false");
    //ls_warning("ammo: %d\n", getAmmo(getAmmoType()));
    if (tank_controls->getFire() && 
            isCannonReady() &&
            getAmmo(getAmmoType()) > 0) {
        fire_sig.emit();
        ammo[getAmmoType()]--;
        //tank_controls->setFire(false);
        reloadCannon();
    }
}
