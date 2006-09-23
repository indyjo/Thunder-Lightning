#include <landscape.h>
#include <interfaces/IGame.h>
#include <modules/engines/rigidengine.h>
#include <modules/model/model.h>

#include "RigidActor.h"

using namespace Collide;

RigidActor::RigidActor(Ptr<IGame> game, Ptr<BoundingGeometry> bgeom)
:	SimpleActor(game),
	Collidable(bgeom),
	gravity(0,0,0)
{
	rigid_engine = new RigidEngine(game);
	setRigidBody(&*rigid_engine);
	setActor(this);
	game->getCollisionMan()->add(this);
	setEngine(rigid_engine);
	rigid_engine->setLinearMomentum(Vector(0,0,0));
}

RigidActor::~RigidActor() {
	if(thegame->getCollisionMan())
		thegame->getCollisionMan()->remove(this);
}

void RigidActor::action() {
	rigid_engine->applyLinearAcceleration(gravity);
	SimpleActor::action();
}

void RigidActor::integrate(float delta_t, Transform * transforms) {
	rigid_engine->integrate(delta_t, transforms);
}

void RigidActor::update(float delta_t, const Transform * new_transforms) {
	rigid_engine->update(delta_t, new_transforms);
}

