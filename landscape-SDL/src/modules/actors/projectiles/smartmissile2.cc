#include "smartmissile2.h"
#include <modules/actors/fx/explosion.h>
#include <modules/actors/RelativeView.h>
#include <modules/clock/clock.h>
#include <modules/engines/missileengine.h>
#include <modules/gunsight/gunsight.h>
#include <modules/model/modelman.h>
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <interfaces/ITerrain.h>
#include <sound.h>
#include <remap.h>

#define BLAST_BEGIN 0.5
#define BLAST_END 2.5
#define BLAST_THRUST 74000.0
#define MIN_EXPLOSION_AGE 1.5f
#define MAX_LIFETIME 30.0f
#define PI 3.14159265358979323846264338327f
#define SCAN_ANGLE (120 * PI / 180.0)
#define MIN_INTERCEPT_SPEED 250.0
#define MAX_INTERCEPT_SPEED 500.0


/**
A missile similiar to the AIM-9 Sidewinder missile.
Data taken from:
http://www.chinfo.navy.mil/navpalib/factfile/missiles/wep-side.html
*/


SmartMissile2::SmartMissile2(Ptr<IGame> thegame, Ptr<IActor> target, Ptr<IActor> source)
:   SimpleActor(thegame), target(target), damage(0), age(0), source(source)
{
    renderer = thegame->getRenderer();
    terrain = thegame->getTerrain();
    setTargetInfo(new TargetInfo(
        "Smart missile", 1.0f, TargetInfo::GUIDED_MISSILE));

    float front_area = PI * 0.13f*0.13f;
    float side_area = 3.0f * 0.63f*0.63f;
    float m = 85.5f;  // wheight in kg
    float l = 2.89f;  // length in m
    float r = 0.065f; // radius in m
    float Iz = 0.5f*m*r*r; 			//  0.18062
    float Ix = m*(l*l/12 + r*r/4);  // 59.59902

    engine = new MissileEngine2(thegame, front_area, side_area);
    //engine = new RigidEngine(thegame);
    //engine->construct(m, Ix, Ix, Iz);
    //engine->construct(m, Ix, Ix, Ix);
    engine->construct(m, 60, 60, 20);
    setEngine(engine);

    d_error_old = omega_error_old = Vector(0,0,0);

    // Prepare collidable
    setBoundingGeometry(new Collide::BoundingGeometry(1,1));
    getBoundingGeometry()->setBoundingRadius(0.0f * l);
    setRigidBody(&*engine);
    setActor(this);
    
    engine_sound_src = thegame->getSoundMan()->requestSource();
    engine_sound_src->setPosition(getLocation());
    engine_sound_src->setVelocity(getMovementVector());
    engine_sound_src->setLooping(true);
    //engine_sound_src->setGain(0.01);
    engine_sound_src->play(thegame->getSoundMan()->querySound(
            thegame->getConfig()->query("Missile_engine_sound")));

    thegame->getCollisionMan()->add(this);
    
    setModel(thegame->getModelMan()->query(
    	thegame->getConfig()->query("SmartMissile_model")));
    
  	delta_omega_old = Vector(-1,2,-3);
}


void SmartMissile2::action()
{
    // If too old -> explode
    if (age > MAX_LIFETIME) {
        ls_message("killed by MAX_LIFETIME.\n");
        explode();
        return;
    }

    // If target dead -> clear target
    if (target && target->getState() == IActor::DEAD)
        target = 0;

    // Do the engine blast
    engine_sound_src->setPosition(getLocation());
    engine_sound_src->setVelocity(getMovementVector());
    if(age > BLAST_BEGIN && age < BLAST_END) {
        engine->applyForce(getFrontVector()*BLAST_THRUST);
        engine_sound_src->setGain(1);
    } else {
    	engine_sound_src->setGain(0.2);
    }
    
    if (target) interceptTarget();

    Vector p_old = getLocation();
    SimpleActor::action();
    Vector p = getLocation();

    if (terrain->lineCollides(p_old, p, &p)) {
        //SmokeColumn::PuffParams pparams;
        //pparams.color = Vector(35, 30, 25);
        //thegame->addActor(
        //        new SmokeColumn(thegame, p, SmokeColumn::Params(), pparams));
        ls_message("killed by collision with ground.\n");
        ls_message("location: "); getLocation().dump();
        explode();
    }
}


void SmartMissile2::shoot(
        const Vector &pos,
        const Vector &vec,
        const Vector &dir)
{
    engine->setLocation(pos);
    engine->setMovementVector(vec);
    Vector front = dir;
    Vector right = (Vector(0,1,0) % front).normalize();
    Vector up = (front % right).normalize();
    engine->setOrientation(up, right, front);

    Ptr<SmokeTrail> smoketrail =
            new SmokeTrail(thegame);
    smoketrail->follow(this);
    thegame->addActor(smoketrail);
    age = 0;

    Ptr<SoundMan> soundman = thegame->getSoundMan();
    Ptr<SoundSource> soundsource = soundman->requestSource();
    if (soundsource) {
        soundsource->setPosition(pos);
        //soundsource->play( soundman->querySound("missile-shoot-1.wav") );
    }

    if (target) {
        rendezvous.updateSource(pos, vec, Vector(0,0,0));
        rendezvous.updateTarget(
                target->getLocation(),
                target->getMovementVector(),
                Vector(0,0,0));
    }
}

Ptr<IActor> SmartMissile2::getSource() {
	return source;
}

int SmartMissile2::getNumViews() { return 5; }

Ptr<IView> SmartMissile2::getView(int n) { 
    Ptr<FlexibleGunsight> gunsight = new FlexibleGunsight(thegame);
    gunsight->addDebugInfo(thegame, this);
    //gunsight->addFlightModules(thegame, flight_info);
    gunsight->addBasicCrosshairs();
    //gunsight->addTargeting(this, targeter);
    gunsight->addDirectionOfFlight(this);
    if (target) gunsight->addInterception(this,target);
	switch(n) {
	case 0:
		return new RelativeView(
            this,
            Vector(0,0,0),
            Vector(1,0,0),
            Vector(0,1,0),
            Vector(0,0,1), gunsight);
    case 1:
    	return new RelativeView(
            this,
            Vector(0,0,0),
            Vector(-1,0,0),
            Vector(0,1,0),
            Vector(0,0,-1));
    case 2:
    	return new RelativeView(
            this,
            Vector(0,0,0),
            Vector(0,0,1),
            Vector(0,1,0),
            Vector(-1,0,0));
    case 3:
    	return new RelativeView(
            this,
            Vector(0,0,0),
            Vector(0,0,-1),
            Vector(0,1,0),
            Vector(1,0,0));
	case 4:
		return new RelativeView(
            this,
            Vector(0,5,-25),
            Vector(1,0,0),
            Vector(0,1,0),
            Vector(0,0,1), gunsight);
    default:
    	return 0;
	}
}


void SmartMissile2::hitTarget(float damage)
{
    if ((this->damage+=damage) > 0.1) explode();
}

#define RAND ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)
#define RAND_POS ((float) rand() / (float) RAND_MAX)

#define DAMAGE_RADIUS 50.0f
#define MAX_DAMAGE 1.5f
void SmartMissile2::explode()
{
    thegame->addActor(new Explosion(thegame, getLocation(), 10.0));
    state = DEAD;
    //ls_message("state=DEAD\n");
    thegame->getCollisionMan()->remove(this);

    typedef IActorStage::ActorVector Actors;
    typedef Actors::iterator Iter;
    
    Actors actors;
    thegame->queryActorsInSphere(actors, getLocation(), DAMAGE_RADIUS);
    for(Iter i=actors.begin(); i!=actors.end(); ++i) {
    	const Ptr<IActor> & actor = *i;
    	if (actor == this) continue;
        float dist = (actor->getLocation() - getLocation()).length();
        float damage = (1.0 - dist / DAMAGE_RADIUS);
        damage *= MAX_DAMAGE;
        actor->applyDamage(damage, 0, this);
    }
    engine_sound_src->stop();
}


void SmartMissile2::integrate(float delta_t, Transform * transforms) {
    engine->integrate(delta_t, transforms);
}

void SmartMissile2::update(float delta_t, const Transform * new_transforms) {
    engine->update(delta_t, new_transforms);
}

void SmartMissile2::collide(const Collide::Contact & c) {
    Ptr<Collidable> partner;
    if (c.collidables[0] == this)
        partner = c.collidables[1];
    else
        partner = c.collidables[0];
    Ptr<IActor> a = partner->getActor();
    if (age <= MIN_EXPLOSION_AGE) return;
    //if (a) a->applyDamage(MAX_DAMAGE);
    explode();
}

Vector SmartMissile2::getDesiredDirection() {
	Vector p = getLocation();
	Vector v = getMovementVector();
	
    Vector target_p = target->getLocation();
    Vector target_v = target->getMovementVector();
    
    Vector target_dir = (target_p-p).normalize();
    Vector target_vrel = target_v - target_dir*(target_dir*target_v);
    float target_vrel2 = target_vrel.lengthSquare();
    float v2 = v.lengthSquare();
    if (target_vrel2 > v2) target_vrel2 = v2;
    Vector desired_v = target_vrel + target_dir * sqrt(v2 - target_vrel2);
    //return (desired_v).normalize();
    return (desired_v - 0.5*v).normalize();
}

Vector SmartMissile2::getDesiredOmega(const Vector &desired_direction) {
	Vector d = getMovementVector().normalize();
    Vector axis = (d % desired_direction).normalize();
    return axis * acos(d*desired_direction);
}

void SmartMissile2::interceptTarget() {
    float delta_t = thegame->getClock()->getStepDelta();
    age += delta_t;

    Vector target_p = target->getLocation();
    Vector target_v = target->getMovementVector();
    Vector p = getLocation();
    Vector d = getFrontVector();

    // Aiming
    if ((target_p-p).normalize() * d < cos(SCAN_ANGLE)) {
        if (age>MIN_EXPLOSION_AGE) {
            ls_message("killed when target got out of SCAN_ANGLE.\n");
            explode();
        }
        return;
    }
	Vector desired_direction = getDesiredDirection();
	Vector desired_omega = getDesiredOmega(desired_direction);
    
    Vector omega = engine->getAngularVelocity();
    Vector delta_omega = desired_omega - omega;
    Vector d_delta_omega =
    	(delta_omega-delta_omega_old) / delta_t;
    // Magic initialization value
    if ((delta_omega_old-Vector(-1,2,-3)).lengthSquare() < 1e-10)
    	d_delta_omega = Vector(0,0,0);
    delta_omega_old = delta_omega;
    
    	
    Ptr<IConfig> config = thegame->getConfig();
    //ls_message("delta_omega: "); delta_omega.dump();
    //ls_message("d_delta_omega: "); d_delta_omega.dump();
    //ls_message("Applying angular accel: ");
    (config->queryFloat("Missile_Ka",1)*delta_omega
    	+config->queryFloat("Missile_Kb",1)*d_delta_omega).dump();
    engine->applyAngularAcceleration(
    	config->queryFloat("Missile_Ka",1)*delta_omega
    	+config->queryFloat("Missile_Kb",1)*d_delta_omega);
}
