#ifndef COLLIDABLE_H
#define COLLIDABLE_H

#include <object.h>
#include <interfaces/IActor.h>
#include <modules/math/Transform.h>
#include <modules/physics/RigidBody.h>

#include "BoundingGeometry.h"

struct IActor;
struct RigidBody;

namespace Collide {

struct BoundingGeometry;
struct Contact;

class Collidable : virtual public Object{
    Ptr<BoundingGeometry> bounding;
    IActor *actor;
    RigidBody *rigid;
    Ptr<Collidable> ncparent;
protected:
    inline Collidable(Ptr<BoundingGeometry> b=0,
                      RigidBody *r=0, IActor *a=0,
                      Ptr<Collidable> ncparent=0)
    :   bounding(b), rigid(r), actor(a), ncparent(ncparent)
    { }
protected:
    inline void setBoundingGeometry(Ptr<BoundingGeometry> b) {
        this->bounding = b;
    }
    inline void setRigidBody(RigidBody *r) {
        this->rigid = r;
    }
    inline void setActor(IActor *a) {
        this->actor = a;
    }
    
    inline Collidable* getNoCollideRoot() {
    	Collidable *root = this;
    	while(root->ncparent)
    		root = ptr(root->ncparent);
    	return root;
    }
    	
public:
    /// returns associated bounding geometry.
    inline Ptr<BoundingGeometry>
    getBoundingGeometry() { return bounding; }

    /// returns an associated rigid body.
    /// Warning: this can be null!
    inline RigidBody *getRigid() { return rigid; }

    /// returns an associated actor.
    /// Warning: This can be null!
    inline IActor *getActor() { return actor; }
    
    /// Sets the collidable's parent in the no-collide tree.
    /// If the goal is to disconnect the collidable from the
    /// no-collide tree, set parent to 0
    inline void setNoCollideParent(Ptr<Collidable> parent) {
    	ncparent = parent;
    }
    /// Checks whether this and other belong to the same no-collide tree
    inline bool noCollideWith(Ptr<Collidable> & other) {
    	return getNoCollideRoot() == other->getNoCollideRoot();
    }

    // These three methods have to be implemented by the derived class

    // Query the state of the transforms after delta_t seconds.
    // If delta_t == 0 this just copies the current state
    // negative delta_t values are forbidden
    virtual void integrate(float delta_t, Transform * transforms) = 0;

    // Update the state of the object and signal that delta_t seconds have passed
    // since the last update.
    virtual void update(float delta_t, const Transform * new_transforms) = 0;

    // Signal the object that a collision is about to happen.
    // The purpose of this method is not to calculate the rigid body collision response
    // (i.e. an impulse) but for the game logic to afflict damage to the partner or
    // score points and so on.
    // The default implementation is to do nothing
    virtual void collide(const Contact &);
};
	
} // namespace Collide

#endif
