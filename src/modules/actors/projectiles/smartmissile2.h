#ifndef SMARTMISSILE2_H
#define SMARTMISSILE2_H

#include <landscape.h>
#include <interfaces/IProjectile.h>
#include <modules/actors/fx/spark.h>
#include <modules/actors/fx/smoketrail.h>
#include <modules/actors/fx/smokecolumn.h>
#include <modules/actors/simpleactor.h>
#include <modules/collide/CollisionManager.h>
#include <modules/math/Rendezvous.h>
#include <modules/math/Differential.h>

class MissileEngine2;
class RigidEngine;
class SoundSource;

class SmartMissile2: public SimpleActor, public Collide::Collidable, public IProjectile
{
public:
    SmartMissile2(Ptr<IGame> thegame, Ptr<IActor> target, Ptr<IActor> source=0);

    virtual void action();

    virtual void shoot(const Vector &pos, const Vector &vec, const Vector &dir);
    virtual Ptr<IActor> getSource();
    virtual Ptr<Collide::Collidable> asCollidable();

    virtual void hitTarget(float damage);
    
    virtual void integrate(float delta_t, Transform * transforms);
    virtual void update(float delta_t, const Transform * new_transforms);
    virtual void collide(const Collide::Contact & c);

    virtual int getNumViews();
    virtual Ptr<IView> getView(int n);


private:
    void explode();
    void interceptTarget(float delta_t);

private:
    float age;
    JRenderer *renderer;
    Ptr<IActor> target;
    Ptr<ITerrain> terrain;
    Rendezvous rendezvous;
    float damage;
    
    Ptr<RigidEngine> engine;
    Ptr<SoundSource> engine_sound_src;
    Ptr<IActor> source;
    
    Integral<Vector> I_error_dt;
    Differential<Vector> d_error_dt;
};


#endif
