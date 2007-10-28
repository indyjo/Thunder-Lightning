#ifndef CARRIER_H
#define CARRIER_H

#include <tnl.h>
#include <modules/actors/simpleactor.h>
#include <modules/collide/Collidable.h>
#include <modules/engines/rigidengine.h>

class Turret;
class Armament;
struct IWeapon;

class Carrier : public SimpleActor, virtual public Collide::Collidable, virtual public SigObject {
public:
    Carrier(Ptr<IGame> thegame, IoObject * io_peer=0);
    //~Carrier();

    virtual void onLinked();
    virtual void onUnlinked();
    
    virtual void action();

    virtual bool hasControlMode(ControlMode);
    virtual void setControlMode(ControlMode);
    
    virtual int getNumViews();
    virtual Ptr<IView> getView(int n);

    // Collidable implementation
    virtual void integrate(float delta_t, Transform * transforms);
    virtual void update(float delta_t, const Transform * new_transforms);
    
private:
    void updateDerivedObjects();

    Ptr<RigidEngine> engine;
    Ptr<Turret> main_turret;
    enum ControlTarget {
        CARRIER,
        MAIN_TURRET
    } control_target;
    void setControlTarget(ControlTarget c);
    void carrierGunFired(Ptr<IWeapon>);
    
    Ptr<Armament> main_turret_armament;
};




#endif

