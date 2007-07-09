#include <algorithm>

#include <interfaces/IConfig.h>

#include <modules/clock/clock.h>
#include <modules/engines/effectors.h>
#include <modules/engines/rigidengine.h>
#include <modules/model/modelman.h>
#include <sound.h>

#include "smartmissile2.h"

#define PI 3.14159265358979323846264338327f
#define SCAN_ANGLE (120 * PI / 180.0)

/**
A missile similiar to the AIM-9 Sidewinder missile.
Data taken from:
http://www.chinfo.navy.mil/navpalib/factfile/missiles/wep-side.html
*/


SmartMissile2::SmartMissile2(Ptr<IGame> thegame, Ptr<IActor> target, Ptr<IActor> source)
:   Missile(thegame, source, "SmartMissile"), target(target)
{
    setTargetInfo(new TargetInfo(
        "Interceptor Missile", 1.0f, TargetInfo::GUIDED_MISSILE));
    engine->addEffector(new Effectors::MissileControl);
}


void SmartMissile2::action()
{
    Missile::action();
    
    // If target dead -> clear target
    if (target && target->getState() == IActor::DEAD)
        target = 0;
        
    // If no target and old enough -> explode
    if (!target && age > min_explosion_age) {
        explode();
    }

    float delta_t = thegame->getClock()->getStepDelta();
    if (target) interceptTarget(delta_t);
}

void SmartMissile2::interceptTarget(float delta_t) {
    Vector target_p = target->getLocation();
    Vector target_v = target->getMovementVector();
    Vector p = getLocation();
    Vector v = getMovementVector();
    
    // Proximity fuse, kind of
    if ((target_p-p) * v < 0) {
        if (age>min_explosion_age) {
            ls_message("killed when angle to target got > 90\n");
            explode();
        }
        return;
    }
    
    Vector los = target_p - p;
    Vector d_los = target_v - v;
    Vector los_norm = Vector(los).normalize();
    Vector d_los_perceived = (d_los - (los_norm * d_los) * los_norm) / los.length();
    Vector los_rotation = los_norm % d_los_perceived;
    los_rotation.normalize();
    los_rotation *= atan(d_los_perceived.length());
    
    Vector error = los_rotation;
    Vector d_error = d_error_dt.differentiate(error, delta_t);
    Vector I_error = I_error_dt.integrate(error, delta_t);

    Ptr<IConfig> config = thegame->getConfig();
    getControls()->setVector("angular_accel",
        config->queryFloat("SmartMissile_Kp",1)*error
        + config->queryFloat("SmartMissile_Ki",1)*I_error
        + config->queryFloat("SmartMissile_Kd",1)*d_error
        );
}

