#include "ObserverEngine.h"

ObserverEngine::ObserverEngine(Ptr<IGame> game)
    :clock(game->getClock())
    ,v(0,0,0)
    ,xform(Quaternion(1,0,0,0), Vector(0))
{
    controls = new DataNode;
}

void ObserverEngine::setControls(Ptr<DataNode> controls) { this->controls = controls; }

void ObserverEngine::run() {
    float delta_t = clock->getStepDelta();
    
    v += Vector(
        delta_t*controls->getFloat("accel_x"),
        delta_t*controls->getFloat("accel_y"),
        delta_t*controls->getFloat("accel_z"));
        
    v[0] = controls->getBool("stop_x")?0.0f:v[0];
    v[1] = controls->getBool("stop_y")?0.0f:v[1];
    v[2] = controls->getBool("stop_z")?0.0f:v[2];
        
    Quaternion rotx = Quaternion::Rotation(Vector(1,0,0), delta_t*controls->getFloat("rotate_x"));
    Quaternion roty = Quaternion::Rotation(Vector(0,1,0), delta_t*controls->getFloat("rotate_y"));
    Quaternion rotz = Quaternion::Rotation(Vector(0,0,1), -delta_t*getRightVector()[1]);
    xform.quat() = xform.quat() * roty * rotx * rotz;
    
    xform.vec() = xform.vec() + xform.quat().rot(delta_t*v);
    
    controls->setBool("stop_x", false);
    controls->setBool("stop_y", false);
    controls->setBool("stop_z", false);
}

// IPositionProvider
Vector ObserverEngine::getLocation() { return xform.vec(); }
Vector ObserverEngine::getFrontVector() { return xform.quat().rot(Vector(0,0,1)); }
Vector ObserverEngine::getRightVector() { return xform.quat().rot(Vector(1,0,0)); }
Vector ObserverEngine::getUpVector() { return xform.quat().rot(Vector(0,1,0)); }
void ObserverEngine::getOrientation(Vector * up, Vector * right, Vector * front) {
    *front = xform.quat().rot(Vector(0,0,1));
    *right = xform.quat().rot(Vector(1,0,0));
    *up = xform.quat().rot(Vector(0,1,0));
    /*
    ls_message("* right:"); right->dump();
    ls_message("     up:"); up->dump();
    ls_message("  front:"); front->dump();
    */
}

// IMovementProvider
Vector ObserverEngine::getMovementVector() { return v; }

// IPositionReceiver
void ObserverEngine::setLocation(const Vector &p) {
    xform = Transform(xform.quat(), p);
}

void ObserverEngine::setOrientation(const Vector & up,
                            const Vector & right,
                            const Vector & front)
{
    Matrix3 orient= MatrixFromColumns(right, up, front);
    xform.quat().fromMatrix(orient);
}

// IMovementReceiver
void ObserverEngine::setMovementVector(const Vector &) { }


