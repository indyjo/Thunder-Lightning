#ifndef SMARTMISSILE2_H
#define SMARTMISSILE2_H

#include <landscape.h>
#include <modules/actors/fx/spark.h>
#include <modules/actors/fx/smoketrail.h>
#include <modules/actors/fx/smokecolumn.h>
#include <modules/actors/simpleactor.h>
#include <modules/collide/CollisionManager.h>
#include <modules/math/Rendezvous.h>


class MissileEngine2;

class SmartMissile2: public SimpleActor, public Collide::Collidable
{
public:
    SmartMissile2(Ptr<IGame> thegame, Ptr<IActor> target);

    virtual void action();

    virtual void draw();

    virtual void shoot(const Vector &pos, const Vector &vec, const Vector &dir);

    virtual void hitTarget(float damage);
    
    virtual void integrate(float delta_t, Transform * transforms);
    virtual void update(float delta_t, const Transform * new_transforms);
    virtual void collide(const Collide::Contact & c);

private:
    void explode();

private:
    float age;
    JRenderer *renderer;
    Ptr<IActor> target;
    Ptr<ITerrain> terrain;
    Rendezvous rendezvous;
    float damage;
    Ptr<MissileEngine2> engine;
    Vector d_error_old, omega_error_old;
};


#endif
