#include <modules/actors/fx/smokecolumn.h>
#include <modules/collide/CollisionManager.h>
#include <modules/clock/clock.h>
#include <modules/drawing/lensflare.h>
#include <modules/engines/effectors.h>
#include <modules/engines/rigidengine.h>
#include <interfaces/ICamera.h>
#include <interfaces/ITerrain.h>

#include "Decoy.h"

#define EARTH_GRAVITY 9.81

Decoy::Decoy(Ptr<IGame> thegame, Ptr<IActor>, Ptr<IActor> source)
:   SimpleActor(thegame) , age(0), source(source)
{
    setTargetInfo( new TargetInfo("Decoy flare", 1, TargetInfo::DECOY_FLARE));

    engine = new RigidEngine(thegame);
    engine->construct(0.5f, 2e-5, 2e-5, 2e-5);
    //engine->construct(, 1, 1, 1);
    setEngine(engine);
    
    engine->addEffector(Effectors::Gravity::getInstance());
    engine->addEffector(new Effectors::Drag(0.02));

    // Prepare collidable
    setBoundingGeometry(new Collide::BoundingGeometry(1,1));
    getBoundingGeometry()->setBoundingRadius(0.1f);
    setRigidBody(&*engine);
    setActor(this);
}

Decoy::~Decoy() {
};

void Decoy::onLinked() {
    SimpleActor::onLinked();
    thegame->getCollisionMan()->add(this);
    
    SmokeColumn::Params params;
    params.interval=0.1;
    params.ttl=8;
    SmokeColumn::PuffParams puffparams;
    puffparams.start_size=1.5f;
    puffparams.end_size=3.5f;
    puffparams.color=Vector(0.9,0.9,0.9);
    puffparams.ttl=Interval(3,5);
    puffparams.pos_deviation=0.2;
    puffparams.direction_deviation=0.5;
    puffparams.fadeout=2.0;
    Ptr<FollowingSmokeColumn> smoke =
            new FollowingSmokeColumn(thegame, params, puffparams);
    smoke->follow(this);
    thegame->addActor(smoke);
}

void Decoy::onUnlinked() {
    SimpleActor::onUnlinked();
    thegame->getCollisionMan()->remove(this);
}

void Decoy::action()
{
    double time_in_secs = thegame->getClock()->getStepDelta();

    age += time_in_secs;

    SimpleActor::action();
}

void Decoy::draw()
{
    FlareParams params(thegame->getTexMan(), thegame->getConfig());
    params.half_size = 1;
    drawLensFlare(thegame->getRenderer(),
                  getLocation(),
                  thegame->getCamera(),
                  params,
                  1,
                  age
                 );

}

void Decoy::shoot(const Vector &pos, const Vector &vec, const Vector &dir)
{
    setLocation(pos);
    setMovementVector(vec);
}

Ptr<IActor> Decoy::getSource() {
	return source;
}

Ptr<Collide::Collidable> Decoy::asCollidable() {
    return this;
}

void Decoy::integrate(float delta_t, Transform * transforms) {
    engine->integrate(delta_t, transforms);
}

void Decoy::update(float delta_t, const Transform * new_transforms) {
    // And a cheap-ass terrain collision test
    Vector p_old = getLocation();
    Vector p = new_transforms[0].vec();

    if (p[1] <= 0 && p_old[1] > 0) {
        p = p_old + (p-p_old)* (p_old[1] / (p_old[1]-p[1]));
        setLocation(p);
        die();
        return;
    } else if (thegame->getTerrain()->lineCollides(p_old, p, &p)) {
        setLocation(p);
        die();
        return;
    }
    
    engine->update(delta_t, new_transforms);
}

void Decoy::collide(const Collide::Contact & c) {
    Ptr<Collidable> partner;
    int domain;
    if (c.collidables[0] == this) {
        partner = c.collidables[1];
        domain = c.domains[1];
    } else {
        partner = c.collidables[0];
        domain = c.domains[0];
    }
    Ptr<IActor> a = c.collidables[1]->getActor();
    a->applyDamage(0.1f, c.domains[1], this);
}

void Decoy::die() {
    state = DEAD;
}

