#include "smartmissile.h"
#include <modules/actors/fx/explosion.h>
#include <modules/engines/missileengine.h>
#include <modules/model/modelman.h>
#include <interfaces/IActorStage.h>
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <interfaces/ITerrain.h>
#include <sound.h>

#define BLAST_BEGIN 1.0
#define BLAST_END 30.0
#define BLAST_THRUST 74000.0
#define MIN_EXPLOSION_AGE 1.0f
#define MAX_LIFETIME 40.0
#define PI 3.14159265358979323846264338327f
#define SCAN_ANGLE (140.0 * PI / 180.0)
#define MIN_INTERCEPT_SPEED 250.0
        

SmartMissile::SmartMissile(Ptr<IGame> thegame, Ptr<IActor> target, Ptr<IActor> source)
: SimpleActor(thegame), target(target), damage(0), source(source)
{
    renderer = thegame->getRenderer();
    terrain = thegame->getTerrain();
    setTargetInfo(new TargetInfo(
        "Smart missile", 1.0f, TargetInfo::GUIDED_MISSILE));
    engine = new SmartMissileEngine(thegame);
    setEngine(engine);
    if (target) {
        marker = new TargetMarker(thegame);
        Vector target_pos = target->getLocation();
        marker->setPos(target_pos);
        thegame->addActor(marker);
    }

    engine_sound_src = thegame->getSoundMan()->requestSource();
    engine_sound_src->setPosition(getLocation());
    engine_sound_src->setVelocity(getMovementVector());
    engine_sound_src->setLooping(true);
    //engine_sound_src->setGain(0.01);
    engine_sound_src->play(thegame->getSoundMan()->querySound(
            thegame->getConfig()->query("Missile_engine_sound")));

    setModel(thegame->getModelMan()->query(
    	thegame->getConfig()->query("SmartMissile_model")));
}



#define MAX_ROTATION_SPEED (90.0f*PI/180.0f)
void SmartMissile::action()
{
    double delta_t = thegame->getTimeDelta() / 1000.0;
    age += delta_t;

    if (age > MAX_LIFETIME) {
        ls_message("killed by MAX_LIFETIME.\n");
        explode();
        return;
    }

    if (target && target->getState() == IActor::DEAD) {
        marker->kill();
        target = 0;
    }

    Vector p = getLocation();
    Vector v = getMovementVector();
    Vector d = engine->getDirection();
    engine_sound_src->setPosition(getLocation());
    engine_sound_src->setVelocity(getMovementVector());
    if (age < BLAST_BEGIN) {
    	engine_sound_src->setGain(age/BLAST_BEGIN);
    	//engine_sound_src->setPitch(0.2 + 0.8* age/BLAST_BEGIN);
    }

    // Aiming
    if (target) {
        Vector target_pos = target->getLocation();
        Vector target_speed = target->getMovementVector();

        if (age>MIN_EXPLOSION_AGE
                && target->getTargetInfo()
                && (target_pos - p).length() <
                    target->getTargetInfo()->getTargetSize()) {
            ls_message("killed by collision with target.\n");
            explode();
            return;
        }

        rendezvous.updateSource(p, Vector(0,0,0), Vector(0,0,0));
        rendezvous.updateTarget(delta_t, target_pos, target_speed);
        rendezvous.setVelocity(v.length());
        Vector rendezvous_point;
        if (v.length() > MIN_INTERCEPT_SPEED) {
            rendezvous_point = rendezvous.calculate();
            float dist = (target_pos-p).length();
            float d1 = 3000;
            float d2 = 1500;
            if (dist>d1) {
            	rendezvous_point = target_pos;
            } else if (dist>d2) {
            	float t = (dist-d2) / (d1-d2);
            	//TODO: Not sure about the t*=t line. arrrrgh.
				t*=t;
            	rendezvous_point = (1-t)*rendezvous_point + t*target_pos;
            }
        } else {
            rendezvous_point = target_pos;
        }

        Vector target_direction = (target_pos-p).normalize();

        if (target_direction * Vector(v).normalize() > cos(SCAN_ANGLE)) {
            engine->setTarget(rendezvous_point);
            marker->setPos(rendezvous_point);
        } else {
            if (age>MIN_EXPLOSION_AGE) {
                ls_message("killed when target got out of SCAN_ANGLE.\n");
                explode();
                return;
            }
        }
    }

    if(age > BLAST_BEGIN && age < BLAST_END) engine->setThrust(BLAST_THRUST);
    else engine->setThrust(0);
    
    Vector p_old = p;
    SimpleActor::action();
    p = getLocation();

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

void SmartMissile::shoot(
        const Vector &pos,
        const Vector &vec,
        const Vector &dir)
{
    engine->setLocation(pos);
    engine->setMovementVector(vec);
    engine->setDirection(dir);
    ls_message("shooting missile pos vec dir: \n");
    engine->getLocation().dump();
    engine->getMovementVector().dump();
    engine->getDirection().dump();

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

Ptr<IActor> SmartMissile::getSource() {
	return source;
}

void SmartMissile::applyDamage(float damage, int domain, Ptr<IProjectile>)
{
    if ((this->damage+=damage) > 0.1) explode();
}

#define MAX_HORIZONTAL_SPEED 600.0
#define MAX_VERTICAL_SPEED 600.0
#define RAND ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)
#define RAND_POS ((float) rand() / (float) RAND_MAX)

#define DAMAGE_RADIUS 80.0f
#define MAX_DAMAGE 2.0f
void SmartMissile::explode()
{
    thegame->addActor(new Explosion(thegame, getLocation(), 10.0));
    if(marker) marker->kill();
    state = DEAD;

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
    
