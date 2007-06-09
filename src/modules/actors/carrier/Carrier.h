#ifndef CARRIER_H
#define CARRIER_H

#include <tnl.h>
#include <modules/actors/simpleactor.h>
#include <modules/collide/Collidable.h>
#include <modules/engines/rigidengine.h>

class Carrier : public SimpleActor, virtual public Collide::Collidable, virtual public SigObject {
public:
    Carrier(Ptr<IGame> thegame, IoObject * io_peer=0);
    //~Carrier();

    virtual void onLinked();
    virtual void onUnlinked();
    
    /*
    virtual void action();

    virtual void draw();
    */

    // Collidable implementation
    virtual void integrate(float delta_t, Transform * transforms);
    virtual void update(float delta_t, const Transform * new_transforms);
private:
    Ptr<RigidEngine> engine;
};




#endif

