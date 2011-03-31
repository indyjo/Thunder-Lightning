#ifndef RIGID_ACTOR_H
#define RIGID_ACTOR_H

#include <modules/collide/CollisionManager.h>
#include <TargetInfo.h>

#include "simpleactor.h"

class RigidEngine;

class RigidActor : public SimpleActor, public Collide::Collidable {
	Ptr<RigidEngine> rigid_engine;
	Vector gravity;
public:
	RigidActor(Ptr<IGame>, Ptr<Collide::BoundingGeometry> bgeom);
	~RigidActor();

    inline Ptr<RigidEngine> getEngine() { return rigid_engine; }
    inline void setGravity(const Vector& v) { gravity=v; }

    virtual void action();
    
    virtual void integrate(float delta_t, Transform * transforms);
    virtual void update(float delta_t, const Transform * new_transforms);
};







#endif

