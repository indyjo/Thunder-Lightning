#ifndef MISSILE_H
#define MISSILE_H

#include <interfaces/IProjectile.h>
#include <modules/actors/simpleactor.h>
#include <modules/collide/CollisionManager.h>
#include <tnl.h>


class RigidEngine;
class SoundSource;
class Sound;

namespace Effectors {
    class Thrust;
}

/// A basic missile class meant for overloading
class Missile: public SimpleActor, public Collide::Collidable, public IProjectile
{
protected:
    /// Constructs a missile with aerodynamics taken from config with prefix cfg_prefix
    Missile(Ptr<IGame> thegame, Ptr<IActor> source=0, const char* cfg_prefix="Missile");
    virtual ~Missile();
    
public:
    virtual void onLinked();
    virtual void onUnlinked();

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

    virtual void explode();
    virtual void shootSparks();


protected:
    JRenderer *renderer;
    Ptr<ITerrain> terrain;
    
    float age;
    float blast_begin, blast_end;
    float max_lifetime, min_explosion_age;

    float damage;
    
    Ptr<RigidEngine> engine;
    Ptr<Effectors::Thrust> thrust;
    Ptr<SoundSource> engine_sound_src;
    Ptr<Sound> engine_sound;
    Ptr<IActor> source;
};



#endif

