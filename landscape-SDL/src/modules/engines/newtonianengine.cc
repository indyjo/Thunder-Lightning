#include "newtonianengine.h"

void NewtonianEngine::update() {
    if ( v[0] == 0 && v[2] == 0) { // singular case
        if (v[1] == 0) { // zero velocity
            up = Vector(0,1,0);
            right = Vector(1,0,0);
            front = Vector(0,0,1);
        } else if (v[1] > 0) { // velocity points straight up
            up = Vector(0,0,-1);
            right = Vector(1,0,0);
            front = Vector(0,1,0);
        } else { // velocity points straight down
            up = Vector(0,0,1);
            right = Vector(1,0,0);
            front = Vector(0,-1,0);
        }
    } else { // normal case
        front = v;
        front.normalize();
        up = Vector(0,1,0);
        right = up % front;
        right.normalize();
        up = front % right;
    }
}

// IEngine
void NewtonianEngine::run() {
    double delta_t = thegame->getTimeDelta() / 1000.0;
    v[1] -= 9.81 * delta_t;
    p += v * delta_t;
    update();
}

// IPositionProvider
Vector NewtonianEngine::getLocation() { return p; }
Vector NewtonianEngine::getFrontVector() { return front; }
Vector NewtonianEngine::getRightVector() { return right; }
Vector NewtonianEngine::getUpVector() { return up; }
void NewtonianEngine::getOrientation(Vector *up, Vector *right, Vector *front) {
    *up = this->up;
    *right = this->right;
    *front = this->front;
}

// IMovementProvider
Vector NewtonianEngine::getMovementVector() { return v; }

// IPositionReceiver
void NewtonianEngine::setLocation(const Vector & new_p) { p = new_p; }
void NewtonianEngine::setOrientation(   const Vector & up,
                                    const Vector & right,
                                    const Vector & front) {
    v = v.length() * front;
    update();
}

// IMovementReceiver
void NewtonianEngine::setMovementVector(const Vector & new_v) { v = new_v; }

