#include "simpleactor.h"
#include <modules/engines/newtonianengine.h>
#include <Faction.h>
#include "RelativeView.h"

SimpleActor::SimpleActor( Ptr<IGame> game)
:   thegame(game), state(ALIVE)
{
    engine = new NewtonianEngine(thegame);
    faction = Faction::basic_factions.none;
    target_info = new TargetInfo(
        "<unnamed SimpleActor>", 1.0f, TargetInfo::NONE);
}


// IActor
Ptr<TargetInfo> SimpleActor::getTargetInfo() { return target_info; }
Ptr<Faction> SimpleActor::getFaction() { return faction; }
void SimpleActor::setFaction(Ptr<Faction> fac) { faction = fac; }
void SimpleActor::action() { engine->run(); }
void SimpleActor::kill() { state = DEAD; }
IActor::State SimpleActor::getState() { return state; }
float SimpleActor::getRelativeDamage() { return 0.0f; }
void SimpleActor::applyDamage(float damage, int domain) { }

int SimpleActor::getNumViews() { return 4; }

Ptr<IView> SimpleActor::getView(int n) { 
	switch(n) {
	case 0:
		return new RelativeView(
            this,
            Vector(0,0,0),
            Vector(1,0,0),
            Vector(0,1,0),
            Vector(0,0,1));
    case 1:
    	return new RelativeView(
            this,
            Vector(0,0,0),
            Vector(-1,0,0),
            Vector(0,1,0),
            Vector(0,0,-1));
    case 2:
    	return new RelativeView(
            this,
            Vector(0,0,0),
            Vector(0,0,1),
            Vector(0,1,0),
            Vector(-1,0,0));
    case 3:
    	return new RelativeView(
            this,
            Vector(0,0,0),
            Vector(0,0,-1),
            Vector(0,1,0),
            Vector(1,0,0));
    default:
    	return 0;
	}
}

bool SimpleActor::hasControlMode(ControlMode m) {
  return m==UNCONTROLLED;
}
void SimpleActor::setControlMode(ControlMode) { }


// IPositionProvider
Vector SimpleActor::getLocation() { return engine->getLocation(); }
Vector SimpleActor::getFrontVector() { return engine->getFrontVector(); }
Vector SimpleActor::getRightVector() { return engine->getRightVector(); }
Vector SimpleActor::getUpVector() { return engine->getUpVector(); }
void SimpleActor::getOrientation(Vector *up, Vector *right, Vector *front) {
    engine->getOrientation(up, right, front);
}

// IMovementProvider
Vector SimpleActor::getMovementVector() { return engine->getMovementVector(); }

// IPositionReceiver
void SimpleActor::setLocation(const Vector & p) { engine->setLocation(p); }
void SimpleActor::setOrientation(   const Vector & up,
                                    const Vector & right,
                                    const Vector & front) {
    engine->setOrientation(up, right, front);
}

// IMovementReceiver
void SimpleActor::setMovementVector(const Vector & v) {
    engine->setMovementVector(v);
}

// IDrawable
void SimpleActor::draw() { }

