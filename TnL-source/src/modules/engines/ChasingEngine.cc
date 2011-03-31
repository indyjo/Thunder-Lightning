#include <cassert>
#include <cmath>
#include "ChasingEngine.h"

ChasingEngine::ChasingEngine(Ptr<IGame> game,
                             Ptr<IPositionProvider> target,
                             float pos_thalf,
                             float rot_thalf,
                             Transform pre_xform,
                             Transform post_xform)
:   clock(game->getClock()),
    target(target),
    pos_thalf(pos_thalf),
    rot_thalf(rot_thalf),
    v(0,0,0),
    pre_xform(pre_xform),
    post_xform(post_xform)
{
    current_xform = getTargetTransform();
    previous_xform = current_xform;
    effective_xform = current_xform * post_xform;
}

void ChasingEngine::setControls(Ptr<DataNode> controls) { }
void ChasingEngine::run() {
    previous_xform = effective_xform;
    Transform target_xform = getTargetTransform();
    if (target_xform.quat().imag() * current_xform.quat().imag() +
        target_xform.quat().real() * current_xform.quat().real() < 0)
    {
        current_xform = Transform(-current_xform.quat(), current_xform.vec());
    }
    float      delta_t = clock->getStepDelta();

    float r = pow(2.0f, -delta_t/rot_thalf);
    float s = pow(2.0f, -delta_t/pos_thalf);
    //current_xform = Transform(current_xform.quat(), current_xform.vec() + delta_t*v);
    current_xform = Transform(
        (r*current_xform.quat() + (1-r)*target_xform.quat()).normalize(),
        s*current_xform.vec()  + (1-s)*target_xform.vec());
    
    //ls_warning("current vec: "); new_x.dump();
    //ls_warning("current quat: %f ",
    //    current_xform.quat().real());
    //current_xform.quat().imag().dump();
    //ls_warning("                          target quat: %f ",
    //    target_xform.quat().real());
    //target_xform.quat().imag().dump();
    
    effective_xform = current_xform * post_xform;
    v = (effective_xform.vec()-previous_xform.vec()) / delta_t;
}

// IPositionProvider
Vector ChasingEngine::getLocation() { return effective_xform.vec(); }
Vector ChasingEngine::getFrontVector() { return effective_xform.quat().rot(Vector(0,0,1)); }
Vector ChasingEngine::getRightVector() { return effective_xform.quat().rot(Vector(1,0,0)); }
Vector ChasingEngine::getUpVector() { return effective_xform.quat().rot(Vector(0,1,0)); }
void ChasingEngine::getOrientation(Vector * up, Vector * right, Vector * front) {
    *front = effective_xform.quat().rot(Vector(0,0,1));
    *right = effective_xform.quat().rot(Vector(1,0,0));
    *up = effective_xform.quat().rot(Vector(0,1,0));
}

// IMovementProvider
Vector ChasingEngine::getMovementVector() { return v; }

// IPositionReceiver
void ChasingEngine::setLocation(const Vector &p) {
    effective_xform = Transform(current_xform.quat(), p);
    current_xform = post_xform.inv()*effective_xform;
    previous_xform = effective_xform;
}

void ChasingEngine::setOrientation(const Vector & up,
                            const Vector & right,
                            const Vector & front)
{
    ls_warning("ChasingEngine::setOrientation not yet implemented.\n");
}

// IMovementReceiver
void ChasingEngine::setMovementVector(const Vector &) { }

Transform ChasingEngine::getTargetTransform() {
    Vector up,right,front;
    target->getOrientation(&up, &right, &front);
    Matrix3 M = MatrixFromColumns(right, up, front);
    Quaternion q;
    q.fromMatrix(M);
    Transform xform(q, target->getLocation());
    return xform*pre_xform;
}
