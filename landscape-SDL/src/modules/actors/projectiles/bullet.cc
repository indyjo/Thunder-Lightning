#include "bullet.h"
#include <modules/actors/fx/explosion.h>
#include <modules/engines/rigidengine.h>
#include <interfaces/ICamera.h>
#include <interfaces/ITerrain.h>

#define EARTH_GRAVITY 9.81
#define MAX_DEVIATION_PER_SECOND 10.0
#define MAX_LIFETIME_SECS 5.0

#define NUM_SPARKS 2
#define MAX_HORIZONTAL_SPEED 50.0
#define MAX_VERTICAL_SPEED 25.0
#define POS_DEVIATION 5.0
#define RAND ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)
#define RAND_POS ((float) rand() / (float) RAND_MAX)


// Value is irrelevant, we only need a unique address
int Bullet::no_collide_tag = 42;

Bullet::Bullet(IGame *thegame, Ptr<IActor> source, float factor)
:   SimpleActor(thegame) , age(0), source(source), factor(factor)
{
    this->renderer = thegame->getRenderer();
    this->terrain = thegame->getTerrain();
    this->camera = thegame->getCamera();
    this->ttl = MAX_LIFETIME_SECS;

    engine = new RigidEngine(thegame);
    engine->construct(0.05f, 2e-6, 2e-6, 2e-6);
    //engine->construct(, 1, 1, 1);
    setEngine(engine);

    // Prepare collidable
    setBoundingGeometry(new Collide::BoundingGeometry(1,1));
    getBoundingGeometry()->setBoundingRadius(0.01f);
    setRigidBody(&*engine);
    setActor(this);
    
    setNoCollideTag(&no_collide_tag);

    thegame->getCollisionMan()->add(this);
}

void Bullet::action()
{
    double time_in_secs = thegame->getTimeDelta() / 1000.0;

    // bullets live for MAX_LIFETIME_SECS seconds
    age += time_in_secs;
    if (age > ttl)
        die();

    Vector p_old = getLocation();
    SimpleActor::action();
    Vector p = getLocation();

    if (terrain->lineCollides(p_old, p, &p))
        explode();
}

void Bullet::draw()
{
    renderer->enableAlphaBlending();
    renderer->setBlendMode(JR_BLENDMODE_ADDITIVE);
    renderer->disableZBufferWriting();
    renderer->setVertexMode(JR_VERTEXMODE_GOURAUD);

    renderer->begin(JR_DRAWMODE_CONNECTED_LINES);
    renderer->setColor(Vector(1,1,0));
    renderer->setAlpha(0);
    renderer->vertex(getLocation());
    renderer->setColor(Vector(1,0.8,0.6));
    renderer->setAlpha(1);
    renderer->vertex(getLocation() + 0.01f*getMovementVector());
    renderer->end();

    renderer->enableZBufferWriting();
    renderer->disableAlphaBlending();
    renderer->setBlendMode(JR_BLENDMODE_BLEND);
}

void Bullet::shoot(const Vector &pos, const Vector &vec, const Vector &dir)
{
    setLocation(pos);
    setMovementVector(vec);
}

Ptr<IActor> Bullet::getSource() {
	return source;
}

void Bullet::integrate(float delta_t, Transform * transforms) {
    engine->integrate(delta_t, transforms);
}

void Bullet::update(float delta_t, const Transform * new_transforms) {
    engine->update(delta_t, new_transforms);
}

void Bullet::collide(const Collide::Contact & c) {
    Ptr<Collidable> partner;
    int domain;
    if (c.collidables[0] == this) {
        partner = c.collidables[1];
        domain = c.domains[1];
    } else {
        partner = c.collidables[0];
        domain = c.domains[0];
    }
    Ptr<IActor> a = partner->getActor();
    if(RAND_POS > 0.4) {
        if (a) a->applyDamage(factor*0.15f, domain, this);
        explode();
    } else if (a) a->applyDamage(factor*0.1f, domain, this);
}

void Bullet::die() {
    state = DEAD;
    thegame->getCollisionMan()->remove(this);
}

void Bullet::explode() {
    Vector vec;
    int i,j;
    Ptr<Spark> spark;

    Vector p = getLocation();

    thegame->addActor(new Explosion(thegame, p, 0.2));
    /*
    for (i=0; i<NUM_SPARKS; i++) {
        vec[0] = MAX_HORIZONTAL_SPEED * RAND;
        vec[1] = MAX_VERTICAL_SPEED * RAND_POS;
        vec[2] = MAX_HORIZONTAL_SPEED * RAND;
        Vector ps = p;// + POS_DEVIATION * Vector(RAND, RAND, RAND);
        spark = new Spark(thegame);
        spark->shoot(ps, vec, Vector(0,0,0));
        thegame->addActor(spark);
    }
    */

    die();
}
