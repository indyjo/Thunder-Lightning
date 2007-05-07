#include "rigidengine.h"
#include <modules/clock/clock.h>

RigidEngine::RigidEngine(Ptr<IGame> thegame)
:   thegame(thegame)
{
    struct RigidBodyState state = {
        Vector(0,0,0),
        Quaternion(1, Vector(0,0,0)),
        Vector(0,0,0),
        Vector(0,0,0)};
    setState(state);
    clearForces();
};

void RigidEngine::setControls(Ptr<DataNode> controls) {
}

void RigidEngine::run() {
    float delta_t = thegame->getClock()->getStepDelta();
    struct RigidBodyState state = getState();
    struct RigidBodyState derivative = getDerivative();
    
    state.P += delta_t * derivative.P;
    state.L += delta_t * derivative.L;

    setState(state);
    clearForces();
}


// IPositionProvider
Vector RigidEngine::getLocation() { return getState().x; }
Vector RigidEngine::getFrontVector() { return getState().q.rot(Vector(0,0,1)); }
Vector RigidEngine::getRightVector() { return getState().q.rot(Vector(1,0,0)); }
Vector RigidEngine::getUpVector()    { return getState().q.rot(Vector(0,1,0)); }
void RigidEngine::getOrientation(Vector *up, Vector *right, Vector *front) {
    *up = getState().q.rot(Vector(0,1,0));
    *right = getState().q.rot(Vector(1,0,0));
    *front = getState().q.rot(Vector(0,0,1));
}

// IMovementProvider
Vector RigidEngine::getMovementVector() { return getLinearVelocity(); }

// IPositionReceiver
void RigidEngine::setLocation(const Vector & new_p) {
    struct RigidBodyState state = getState();
    state.x = new_p;
    setState(state);
}

#define ls_vector(v) (v)[0], (v)[1], (v)[2]

void RigidEngine::setOrientation( const Vector & up,
                                  const Vector & right,
                                  const Vector & front) {
    /*
    ls_message("<RigidEngine::setOrientation>\n");
    ls_message("Requested: %+1.3f %+1.3f %+1.3f right\n"
               "           %+1.3f %+1.3f %+1.3f up\n"
               "           %+1.3f %+1.3f %+1.3f front\n",
               ls_vector(right), ls_vector(up), ls_vector(front));
    */

    struct RigidBodyState state = getState();
    state.q.fromMatrix(MatrixFromColumns(right,up,front));
    setState(state);
    /*
    ls_message("Got:       %+1.3f %+1.3f %+1.3f right\n"
               "           %+1.3f %+1.3f %+1.3f up\n"
               "           %+1.3f %+1.3f %+1.3f front\n",
               ls_vector(state.q.rot(Vector(1,0,0))),
               ls_vector(state.q.rot(Vector(0,1,0))),
               ls_vector(state.q.rot(Vector(0,0,1))));
    ls_message("</RigidEngine::setOrientation>\n");
    */
}

// IMovementReceiver
void RigidEngine::setMovementVector(const Vector & new_v) {
    struct RigidBodyState state = getState();
    state.P = Vector(0,0,0);
    setState(state);
    applyLinearVelocity(new_v);
}


void RigidEngine::integrate(float delta_t, Transform * transforms) {
    struct RigidBodyState state = getState();
    struct RigidBodyState derivative = getDerivative();
    state.x += delta_t * derivative.x;
    state.q += delta_t * derivative.q;
    transforms[0] = Transform(state.q.normalize(), state.x);
}

void RigidEngine::update(float delta_t, const Transform * new_transforms) {
    struct RigidBodyState state = getState();
    state.x = new_transforms[0].vec();
    state.q = new_transforms[0].quat();
    setState(state);
}

