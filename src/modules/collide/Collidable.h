#ifndef COLLIDABLE_H
#define COLLIDABLE_H

#include <object.h>
#include <interfaces/IActor.h>
#include <modules/math/Transform.h>
#include <modules/physics/RigidBody.h>

#include "BoundingGeometry.h"

struct IActor;
class RigidBody;

namespace Collide {

class BoundingGeometry;
struct Contact;

class Collidable : virtual public Object{
    Ptr<BoundingGeometry> bounding;
    IActor *actor;
    RigidBody *rigid;
    Ptr<Collidable> ncparent, ncpartner;
    void *nctag;
protected:
    inline Collidable(Ptr<BoundingGeometry> b=0,
                      RigidBody *r=0, IActor *a=0,
                      Ptr<Collidable> ncparent=0)
    :   bounding(b), rigid(r), actor(a), ncparent(ncparent), ncpartner(0), nctag(0)
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
    
    /// Sets a single collidable to not collide with. Example: The actor that shot a bullet.
    inline void setNoCollidePartner(Ptr<Collidable> partner) {
        ncpartner = partner;
    }
    
    /// Sets a non-collide tag. All collisions between Collidables with the same tag
    /// will be ignored. A tag of 0 disables this.
    inline void setNoCollideTag(void * tag) {
        nctag = tag;
    }
    
    /// Checks whether this and other are marked as non-collidable in any of three ways.
    inline bool noCollideWith(Ptr<Collidable> & other) {
        return (nctag && nctag == other->nctag) ||
            other == ncpartner ||
            ptr(other->ncpartner) == this ||
            getNoCollideRoot() == other->getNoCollideRoot();
    }

    // The following three methods have to be implemented by the derived class

    // Query the state of the transforms after delta_t seconds.
    // If delta_t == 0 this just copies the current state
    // negative delta_t values are forbidden.
    // @note For static collidables (without RigidBody) it is allowed to simply
    //       return the current state.
    virtual void integrate(float delta_t, Transform * transforms) = 0;

    // Update the state of the object and signal that delta_t seconds have passed
    // since the last update.
    // @note For static collidables (without RigidBody) it is allowed to ignore
    //       this.
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
