#include <vector>
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <interfaces/ITerrain.h>
#include <modules/actors/fx/explosion.h>
#include <modules/clock/clock.h>
#include <modules/model/modelman.h>
#include <sound.h>
#include "dumbmissile.h"

#define EARTH_GRAVITY 9.81
#define MAX_DEVIATION_PER_SECOND 10.0
#define BLAST_BEGIN 0
#define BLAST_END 2.0
#define BLAST_THRUST 74000.0
#define RESISTANCE_X 0.05
#define RESISTANCE_Y 0.05
#define RESISTANCE_Z 0.0001

DumbMissile::DumbMissile(Ptr<IGame> thegame, Ptr<IActor> source)
    : SimpleActor(thegame), source(source)
{
    this->renderer = thegame->getRenderer();
    this->terrain = thegame->getTerrain();
    this->camera = thegame->getCamera();
    engine = new MissileEngine(thegame);
    setEngine(engine);
    setTargetInfo(new TargetInfo("Dumb missile", 1.0f, TargetInfo::MISSILE));
    setModel(thegame->getModelMan()->query(
    	thegame->getConfig()->query("DumbMissile_model")));
}

void DumbMissile::action()
{
    double delta_t = thegame->getClock()->getStepDelta();
    age += delta_t;
    if(age > BLAST_BEGIN && age < BLAST_END) {
        engine->setThrust(BLAST_THRUST);
    } else {
        engine->setThrust(0);
    }
    
    Vector p_old = getLocation();
    SimpleActor::action();
    Vector p = getLocation();
    
    //thegame->addActor(new Explosion(thegame, p, 0.5));
    if (terrain->lineCollides(p_old, p, &p)) {
        state = DEAD;
        //SmokeColumn::PuffParams pparams;
        
        //pparams.color = Vector(35, 30, 25);
        //thegame->addActor(
        //        new SmokeColumn(thegame, p, SmokeColumn::Params(), pparams));
        shootSparks();
        
        std::vector<Ptr<IActor> > victims;
        float expl_radius=100;
        float damage_radius = 40;
        thegame->queryActorsInSphere(victims, p, expl_radius);
        for(int i=0; i<victims.size(); ++i) {
        	if (!victims[i]->getTargetInfo()) continue;
        	float dist = (victims[i]->getLocation()-p).length()
        		- victims[i]->getTargetInfo()->getTargetSize();
        	if (dist < 0) dist = 0;
        	else if (dist > damage_radius) continue;
        	victims[i]->applyDamage(10*dist / damage_radius, 0, this);
        }
    }
}

void DumbMissile::shoot(
        const Vector &pos,
        const Vector &vec,
        const Vector &dir)
{
    engine->setLocation(pos);
    engine->setMovementVector(vec);
    engine->setDirection(dir);
    
    Ptr<SmokeTrail> smoketrail =
            new SmokeTrail(thegame);
    smoketrail->follow(this);
    thegame->addActor(smoketrail);
    age = 0;
    
    Ptr<SoundMan> soundman = thegame->getSoundMan();
    Ptr<SoundSource> soundsource = soundman->requestSource();
    if (soundsource) {
        soundsource->setPosition(pos);
        //soundsource->setVelocity(vec);
        soundsource->play( soundman->querySound("missile-shoot-1.wav") );
        //soundsource->play( new Sound("/home/jonas/devel/gcc/landscape-SDL/install/share/"
        //        "landscape/sounds/explosion-01.wav"));
    }
}

Ptr<IActor> DumbMissile::getSource() {
	return source;
}


#define NUM_SPARKS 5
#define MAX_HORIZONTAL_SPEED 100.0
#define MAX_VERTICAL_SPEED 100.0
#define RAND ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)
#define RAND_POS ((float) rand() / (float) RAND_MAX)
void DumbMissile::shootSparks()
{
    Vector vec;
    int i,j;
    
    SmokeColumn::Params params;
    SmokeColumn::PuffParams puff_params;
    
    // Setup smoke column parameters so that the puff interval is shorter
    params.ttl = 1.0;
    params.interval = 0.025;
    
    // Setup smoke puff parameters so that their time to live is shorter
    // and they are much smaller then the default
    puff_params.ttl = 2.0;
    puff_params.ttl_deviation = 1.0;
    puff_params.pos_deviation = 0.5;
    puff_params.start_size = 4.0;
    puff_params.end_size = 20.0;
    puff_params.rotations_in_life = 1.5;
    
    thegame->addActor(new Explosion(thegame, getLocation(), 10.0));
    for (i=0; i<NUM_SPARKS; i++) {
        Ptr<Spark> spark(new Spark(thegame));
        vec[0] = MAX_HORIZONTAL_SPEED * RAND;
        vec[1] = MAX_VERTICAL_SPEED * RAND_POS;
        vec[2] = MAX_HORIZONTAL_SPEED * RAND;
        spark->shoot(getLocation(), vec, Vector(vec).normalize());
        thegame->addActor(spark);
        Ptr<FollowingSmokeColumn> smoke = 
                new FollowingSmokeColumn(thegame, params, puff_params);
        smoke->follow(spark);
        thegame->addActor(smoke);
    }
}
    
