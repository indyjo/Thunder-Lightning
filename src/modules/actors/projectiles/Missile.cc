#include <string>
#include <interfaces/IConfig.h>
#include <interfaces/ITerrain.h>

#include <modules/actors/RelativeView.h>
#include <modules/actors/fx/explosion.h>
#include <modules/actors/fx/smokecolumn.h>
#include <modules/actors/fx/smoketrail.h>
#include <modules/actors/fx/spark.h>
#include <modules/clock/clock.h>
#include <modules/engines/ChasingEngine.h>
#include <modules/engines/effectors.h>
#include <modules/engines/rigidengine.h>
#include <modules/gunsight/gunsight.h>
#include <modules/model/modelman.h>
#include <sound.h>

#include "Missile.h"

Missile::Missile(Ptr<IGame> thegame, Ptr<IActor> source, const char *cfg_prefix)
:   SimpleActor(thegame), damage(0), age(0), source(source)
{
    std::string prefix = cfg_prefix;
    
    Ptr<IConfig> cfg = thegame->getConfig();
    blast_begin = cfg->queryFloat(prefix+"_blast_begin", 0.5f);
    blast_end = cfg->queryFloat(prefix+"_blast_end", 5.0f);
    max_lifetime = cfg->queryFloat(prefix+"_max_lifetime",30.0f);
    min_explosion_age = cfg->queryFloat(prefix+"_min_explosion_age", 2.0f);
    
    renderer = thegame->getRenderer();
    terrain = thegame->getTerrain();
    
    setTargetInfo(new TargetInfo(cfg->query(prefix+"_name","Missile"), 1.0f, TargetInfo::MISSILE));

    engine = new RigidEngine(thegame);
    engine->construct(
        cfg->queryFloat(prefix+"_mass", 80),
        cfg->queryFloat(prefix+"_Ixx", 60),
        cfg->queryFloat(prefix+"_Iyy", 60),
        cfg->queryFloat(prefix+"_Izz", 20));
    
    engine->addEffector(Effectors::Gravity::getInstance());
    engine->addEffector(new Effectors::Missile(cfg, cfg_prefix));
    
    this->thrust = new Effectors::Thrust;
    thrust->setMaxForce(Vector(0,0,
        cfg->queryFloat(prefix+"_max_thrust", 10000))); // some random default value, very weak
    engine->addEffector(thrust);
    
    setEngine(engine);

    // Prepare collidable
    setBoundingGeometry(new Collide::BoundingGeometry(1,1));
    getBoundingGeometry()->setBoundingRadius(1.0f);
    setRigidBody(ptr(engine));
    setActor(this);
    
    engine_sound_src = thegame->getSoundMan()->requestSource();
    engine_sound_src->setLooping(true);
    engine_sound = thegame->getSoundMan()->querySound(cfg->query(prefix+"_engine_sound"));
    
    setModel(thegame->getModelMan()->query(
    	cfg->query(prefix+"_model")));
}

Missile::~Missile()
{ }

void Missile::onLinked() {
    SimpleActor::onLinked();
    
    thegame->getCollisionMan()->add(this);
    
    engine_sound_src->setPosition(getLocation());
    engine_sound_src->setVelocity(getMovementVector());
    engine_sound_src->play(engine_sound);

    Ptr<SmokeTrail> smoketrail =
            new SmokeTrail(thegame);
    smoketrail->follow(this);
    thegame->addActor(smoketrail);
}

void Missile::onUnlinked() {
    SimpleActor::onUnlinked();
    
    thegame->getCollisionMan()->remove(this);
    
    engine_sound_src->stop();
}

void Missile::action()
{
    float delta_t = thegame->getClock()->getStepDelta();
    age += delta_t;

    // If too old -> explode
    if (age > max_lifetime) {
        ls_message("killed by MAX_LIFETIME.\n");
        explode();
        return;
    }

    // Do the engine blast
    engine_sound_src->setPosition(getLocation());
    engine_sound_src->setVelocity(getMovementVector());
    if(age > blast_begin && age < blast_end) {
        thrust->setThrottle(1);
        engine_sound_src->setGain(1);
    } else {
        thrust->setThrottle(0);
    	engine_sound_src->setGain(0.2);
    }
}

void Missile::shoot(
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
}

Ptr<IActor> Missile::getSource() {
	return source;
}

Ptr<Collide::Collidable> Missile::asCollidable() {
    return this;
}

void Missile::hitTarget(float damage)
{
    this->damage += damage;
    if (this->damage > 0.1) explode();
}

void Missile::integrate(float delta_t, Transform * transforms) {
    engine->integrate(delta_t, transforms);
}

void Missile::update(float delta_t, const Transform * new_transforms) {
    // And a cheap-ass terrain collision test
    Vector p_old = getLocation();
    Vector p = new_transforms[0].vec();

    if (terrain->lineCollides(p_old, p, &p)) {
        //SmokeColumn::PuffParams pparams;
        //pparams.color = Vector(35, 30, 25);
        //thegame->addActor(
        //        new SmokeColumn(thegame, p, SmokeColumn::Params(), pparams));
        ls_message("Missile: killed by collision with ground.\n");
        explode();
    }
    
    engine->update(delta_t, new_transforms);
}

void Missile::collide(const Collide::Contact & c) {
    ls_message("Missile::collide()\n");
    Ptr<Collidable> partner;
    if (c.collidables[0] == this)
        partner = c.collidables[1];
    else
        partner = c.collidables[0];
    Ptr<IActor> a = partner->getActor();
    if (age <= min_explosion_age) return;
    explode();
}


int Missile::getNumViews() { return 1; }

Ptr<IView> Missile::getView(int n) { 
    Ptr<FlexibleGunsight> gunsight = new FlexibleGunsight(thegame);
    gunsight->addDebugInfo(thegame, this);
    if (n < 4) {
        gunsight->addBasicCrosshairs();
        gunsight->addDirectionOfFlight(this);
    }

    Ptr<SimpleActor> chaser = new SimpleActor(thegame);
    chaser->setEngine(new ChasingEngine(thegame,this, 0.05f, 0.1f));
    thegame->addWeakActor(chaser);

    Ptr<RelativeView> view = new RelativeView(chaser, chaser, this, gunsight);
    view->setViewOffset(
        Vector(3,-2,-1),
        Vector(1,0,0),
        Vector(0,1,0),
        Vector(0,0,1));
    return view;
}


#define RAND ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)
#define RAND_POS ((float) rand() / (float) RAND_MAX)

#define DAMAGE_RADIUS 50.0f
#define MAX_DAMAGE 1.5f
void Missile::explode()
{
    ls_message("Missile::explode()\n");
    thegame->addActor(new Explosion(thegame, getLocation(), 2.5));
    
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
    
    shootSparks();
    
    engine_sound_src->stop();
}

#define NUM_SPARKS 12
#define MAX_HORIZONTAL_SPEED 100.0
#define MAX_VERTICAL_SPEED 100.0
void Missile::shootSparks()
{
    Vector vec;
    int i,j;
    
    SmokeColumn::Params params;
    SmokeColumn::PuffParams puff_params;
    
    // Setup smoke column parameters so that the puff interval is shorter
    params.ttl = 1.0;
    params.interval = 0.1;
    
    // Setup smoke puff parameters so that their time to live is shorter
    // and they are much smaller then the default
    puff_params.ttl = Interval(0.2,0.8);
    puff_params.pos_deviation = 0.5;
    puff_params.start_size = 0.5;
    puff_params.end_size = 2.0;
    puff_params.color = Vector(0.3,0.3,0.4);
    
    for (i=0; i<NUM_SPARKS; i++) {
        Ptr<Spark> spark(new Spark(thegame));
        vec[0] = MAX_HORIZONTAL_SPEED * RAND;
        vec[1] = MAX_VERTICAL_SPEED * RAND;
        vec[2] = MAX_HORIZONTAL_SPEED * RAND;
        spark->shoot(getLocation(), vec, Vector(vec).normalize());
        thegame->addActor(spark);
        Ptr<FollowingSmokeColumn> smoke = 
                new FollowingSmokeColumn(thegame, params, puff_params);
        smoke->follow(spark);
        thegame->addActor(smoke);
    }
}
    
