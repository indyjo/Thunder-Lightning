#ifndef TANK_H
#define TANK_H

#include <tnl.h>
#include <modules/math/Vector.h>
#include <modules/math/Matrix.h>
#include <modules/model/Skeleton.h>
#include <modules/actors/simpleactor.h>
#include <modules/engines/tankengine.h>
#include <modules/engines/controls.h>
#include <modules/weaponsys/Armament.h>
#include <modules/collide/CollisionManager.h>

//struct TankBrain;
class SoundSource;
class EventSheet;
class Targeter;

class Tank : public SimpleActor, virtual public Collide::Collidable, virtual public SigObject {
public:
    Tank(Ptr<IGame> thegame, IoObject * io_peer=0);
    ~Tank();

    virtual void onLinked();
    virtual void onUnlinked();
    
    virtual void action();

    virtual void draw();

    virtual void applyDamage(float damage, int domain, Ptr<IProjectile>);
    
    virtual void setLocation(const Vector & p);

    virtual int getNumViews();
    virtual Ptr<IView> getView(int n);

    void explode();

    virtual bool hasControlMode(ControlMode);
    virtual void setControlMode(ControlMode);

    // Collidable implementation
    virtual void integrate(float delta_t, Transform * transforms);
    virtual void update(float delta_t, const Transform * new_transforms);
    
    /// Updates skeleton and sound objects to current position
    void updateDerivedObjects();

private:
    JRenderer * renderer;
    Ptr<ITerrain> terrain;

    // sound sources
    Ptr<SoundSource> sound_low, sound_high;

    // Tank state
    Ptr<TankControls> tank_controls;
    Ptr<TankEngine> tank_engine;
    //Ptr<TankBrain> brain;
    float damage;
    double age;
};




#endif
