#include "simpleactor.h"
#include <modules/engines/newtonianengine.h>
#include <Faction.h>

SimpleActor::SimpleActor( Ptr<IGame> game)
:   thegame(game), state(ALIVE)
{
    engine = new NewtonianEngine(thegame);
    faction = Faction::basic_factions.none;
    target_info = new TargetInfo(
        "<unnamed SimpleActor>", 1.0f, TargetInfo::CLASS_NONE);

    views.push_back(new RelativeView(
            *this,
            Vector(0,0,0),
            Vector(1,0,0),
            Vector(0,1,0),
            Vector(0,0,1)));
    views.push_back(new RelativeView(
            *this,
            Vector(0,0,0),
            Vector(-1,0,0),
            Vector(0,1,0),
            Vector(0,0,-1)));
    views.push_back(new RelativeView(
            *this,
            Vector(0,0,0),
            Vector(0,0,1),
            Vector(0,1,0),
            Vector(-1,0,0)));
    views.push_back(new RelativeView(
            *this,
            Vector(0,0,0),
            Vector(0,0,-1),
            Vector(0,1,0),
            Vector(1,0,0)));
}


// IActor
Ptr<TargetInfo> SimpleActor::getTargetInfo() { return target_info; }
Ptr<Faction> SimpleActor::getFaction() { return faction; }
void SimpleActor::setFaction(Ptr<Faction> fac) { faction = fac; }
void SimpleActor::action() { engine->run(); }
IActor::State SimpleActor::getState() { return state; }
float SimpleActor::getRelativeDamage() { return 0.0f; }
void SimpleActor::applyDamage(float damage, int domain) { }
int SimpleActor::getNumViews() { return views.size(); }
Ptr<IPositionProvider> SimpleActor::getView(int n) { return views[n]; }

// IPositionProvider
Vector SimpleActor::getLocation() { return engine->getLocation(); }
Vector SimpleActor::getFrontVector() { return engine->getFrontVector(); }
Vector SimpleActor::getRightVector() { return engine->getRightVector(); }
Vector SimpleActor::getUpVector() { return engine->getRightVector(); }
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

// SimpleActor::RelativeView
Vector SimpleActor::RelativeView::getLocation() {
    Vector r, u, f;
    base.getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    return base.getLocation() + M*p;
}
Vector SimpleActor::RelativeView::getFrontVector() {
    Vector r, u, f;
    base.getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    return M*front;
}

Vector SimpleActor::RelativeView::getRightVector() {
    Vector r, u, f;
    base.getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    return M*right;
}
Vector SimpleActor::RelativeView::getUpVector() {
    Vector r, u, f;
    base.getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    return M*up;
}
void SimpleActor::RelativeView::getOrientation
        (Vector *up, Vector *right, Vector *front) {
    Vector r, u, f;
    base.getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    *up = M*this->up;
    *right = M*this->right;
    *front = M*this->front;
}
