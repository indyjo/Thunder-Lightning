#ifndef TNL_DECOY_H
#define TNL_DECOY_H

#include <tnl.h>
#include <interfaces/IProjectile.h>
#include <modules/math/Vector.h>
#include <modules/math/Matrix.h>
#include <modules/actors/simpleactor.h>
#include <modules/collide/Collidable.h>
#include <modules/engines/rigidengine.h>

class Decoy: public IProjectile, public SimpleActor, public Collide::Collidable
{
public:
    Decoy(Ptr<IGame> thegame, Ptr<IActor> target, Ptr<IActor> source=0);
    ~Decoy();

    virtual void onLinked();
    virtual void onUnlinked();
    
    virtual void action();

    virtual void draw();

    virtual void shoot(const Vector &pos, const Vector &vec, const Vector &dir);
    virtual Ptr<IActor> getSource();
    virtual Ptr<Collide::Collidable> asCollidable();
    
    virtual void integrate(float delta_t, Transform * transforms);
    virtual void update(float delta_t, const Transform * new_transforms);
    virtual void collide(const Collide::Contact & c);

private:
    void die();

private:
    Ptr<RigidEngine> engine;
    Ptr<IActor> source;
    float age;
};

#endif

