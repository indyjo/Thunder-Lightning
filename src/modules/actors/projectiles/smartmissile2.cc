#include <algorithm>

#include <interfaces/IConfig.h>

#include <modules/clock/clock.h>
#include <modules/engines/effectors.h>
#include <modules/engines/rigidengine.h>
#include <modules/model/modelman.h>
#include <sound.h>

#include "smartmissile2.h"

#define BLAST_BEGIN 0.5
#define BLAST_END 2.5
#define BLAST_THRUST 74000.0
#define MIN_EXPLOSION_AGE 1.5f
#define MAX_LIFETIME 30.0f
#define PI 3.14159265358979323846264338327f
#define SCAN_ANGLE (120 * PI / 180.0)

/**
A missile similiar to the AIM-9 Sidewinder missile.
Data taken from:
http://www.chinfo.navy.mil/navpalib/factfile/missiles/wep-side.html
*/


SmartMissile2::SmartMissile2(Ptr<IGame> thegame, Ptr<IActor> target, Ptr<IActor> source)
:   Missile(thegame, source), target(target)
{
    blast_begin = BLAST_BEGIN;
    blast_end = BLAST_END;
    max_lifetime = MAX_LIFETIME;
    min_explosion_age = MIN_EXPLOSION_AGE;

    setTargetInfo(new TargetInfo(
        "Interceptor Missile", 1.0f, TargetInfo::GUIDED_MISSILE));

    float m = 85.5f;  // wheight in kg
    float l = 2.89f;  // length in m
    float r = 0.065f; // radius in m
    float Iz = 0.5f*m*r*r; 			//  0.18062
    float Ix = m*(l*l/12 + r*r/4);  // 59.59902

    // As nice as real values are: the above ones bring numerical instability.
    engine->construct(m, 60, 60, 20);
    engine->addEffector(new Effectors::MissileControl);
    thrust->setMaxForce(Vector(0,0,BLAST_THRUST));
    
    setModel(thegame->getModelMan()->query(
    	thegame->getConfig()->query("SmartMissile_model")));
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
    
    //ls_message("missile %p acceleration angular: ", this); los_rotation.dump();
    blast_begin = BLAST_BEGIN;
    blast_end = BLAST_END;
    max_lifetime = MAX_LIFETIME;
    min_explosion_age = MIN_EXPLOSION_AGE;
    

    Ptr<IConfig> config = thegame->getConfig();
    getControls()->setVector("angular_accel",
        config->queryFloat("Missile_Kp",1)*error
        + config->queryFloat("Missile_Ki",1)*I_error
        + config->queryFloat("Missile_Kd",1)*d_error
        );
}

