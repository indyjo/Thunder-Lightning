#include "missileengine.h"
#include <modules/clock/clock.h>
#include <modules/actors/fx/DebugObject.h>
#include <modules/math/SpecialMatrices.h>

///////////////////////////////////////////////////////////////////////////
// MissileEngine
///////////////////////////////////////////////////////////////////////////

void MissileEngine::update() {
    if ( d[0] == 0 && d[2] == 0) { // singular case
        if (d[1] == 0) { // zero velocity
            up = Vector(0,1,0);
            right = Vector(1,0,0);
            front = Vector(0,0,1);
        } else if (d[1] > 0) { // velocity points straight up
            up = Vector(0,0,-1);
            right = Vector(1,0,0);
            front = Vector(0,1,0);
        } else { // velocity points straight down
            up = Vector(0,0,1);
            right = Vector(1,0,0);
            front = Vector(0,-1,0);
        }
    } else { // normal case
        front = d;
        up = Vector(0,1,0);
        right = up % front;
        right.normalize();
        up = front % right;
        up.normalize();
    }
}

// IEngine
void MissileEngine::setControls(Ptr<Controls> controls) {
}

void MissileEngine::run() {
    double delta_t = thegame->getClock()->getStepDelta();

    // calculate drag
    const static float rho = 1.293;
    // frontal
    const static float cw_f = 0.3;
    Vector v_f = d * (v*d); // frontal component of velocity
    float v2 = v_f.lengthSquare();
    Vector drag_force_f = - d * (cw_f * v2 * frontal_area * rho / 2.0);

    // side
    const static float cw_s = 5;
    Vector v_s = v - v_f; // side component of velocity
    v2 = v_s.lengthSquare();
    v_s.normalize();
    Vector drag_force_s = - v_s * (cw_s * v2 * side_area * rho / 2.0);

    // Calculate thrust
    Vector thrust_force = thrust * d;

    // Calculate resulting acceleration
    Vector a = (thrust_force + drag_force_f + drag_force_s) / mass;
    a[1] -= 9.81; // earth gravitation

    v += a * delta_t;
    p += v * delta_t;
}

// IPositionProvider
Vector MissileEngine::getLocation() { return p; }
Vector MissileEngine::getFrontVector() { return front; }
Vector MissileEngine::getRightVector() { return right; }
Vector MissileEngine::getUpVector() { return up; }
void MissileEngine::getOrientation(Vector *up, Vector *right, Vector *front) {
    *up = this->up;
    *right = this->right;
    *front = this->front;
}

// IMovementProvider
Vector MissileEngine::getMovementVector() { return v; }

// IPositionReceiver
void MissileEngine::setLocation(const Vector & new_p) { p = new_p; }
void MissileEngine::setOrientation( const Vector & up,
                                    const Vector & right,
                                    const Vector & front) {
    d = front;
    update();
}

// IMovementReceiver
void MissileEngine::setMovementVector(const Vector & new_v) { v = new_v; }



///////////////////////////////////////////////////////////////////////////
// SmartMissileEngine
///////////////////////////////////////////////////////////////////////////

void SmartMissileEngine::setControls(Ptr<Controls> controls) {
}

void SmartMissileEngine::run() {
    Vector target_dir = target-p;
    if (target_dir.lengthSquare() > 0) {
        double delta_t = thegame->getClock()->getStepDelta();
        target_dir.normalize();

        Vector v_norm = Vector(v).normalize();

        // x is v_norm projected on target_dir
        Vector x = target_dir * (v_norm * target_dir);
        Vector d_target = x + 4.0*(x - v_norm);
        d_target.normalize();

        Vector axis = (d_target % d).normalize();
        float angle = asin(d_target * (d % axis));

        float v_rot = - angle;

        v_rot = std::max( -max_angular_speed, v_rot);
        v_rot = std::min(  max_angular_speed, v_rot);

        Matrix3 rot = RotateAxisMatrix<float>(axis, -v_rot*delta_t);
        setDirection(rot * d);
    }

    MissileEngine::run();
}


/*
void SmartMissileEngine::run() {
    float delta_t = thegame->getClock()->getStepDelta();

    new DebugActor(thegame, p+1*v, "missile V vector", 0.0001f);

    Vector target_dir = (target-p).normalize();
    Vector flight_dir = Vector(v).normalize();

    Vector axis = (flight_dir % target_dir).normalize();
    float angle = acos(flight_dir*target_dir);
    angle = std::min( 1.0f, 3*angle );
    Vector best_dir = RotateAxisMatrix<float>(axis,angle) * flight_dir;
    new DebugActor(thegame, p+100*best_dir, "missile best_dir", 0.0001f);

    Vector old_error = error;
    error = (d % best_dir).normalize() * acos(best_dir*d);
    Vector d_error = (error-old_error)/delta_t;

    Vector acceleration = 2*error + 6*d_error;
    omega += delta_t * acceleration;

    float l = omega.length();
    axis = Vector(omega).normalize();
    Matrix3 rot = RotateAxisMatrix<float>(axis, l*delta_t);
    setDirection(rot * d);

    MissileEngine::run();
}
*/



///////////////////////////////////////////////////////////////////////////
// MissileEngine2
///////////////////////////////////////////////////////////////////////////

/*
void MissileEngine2::run() {
    // air density
    const static float rho = 1.293;

    Vector d = getFrontVector();
    Vector v = getMovementVector();

    // stabilizing torque
    Vector p = getState().x -1.4f * d;
    Vector vs = getVelocityAt(p);
    vs = vs - d * (vs*d);
    applyForceAt(-0.01f * vs * vs.length(), p);

    // frontal drag coefficient
    const static float cw_f = 0.3;
    Vector v_f = d * (v*d); // frontal component of velocity
    float v2 = v_f.lengthSquare();
    Vector drag_force_f = - d * (cw_f * v2 * front_area * rho / 2.0);

    // side drag coefficient
    const static float cw_s = 1.2;
    Vector v_s = v - v_f; // side component of velocity
    v2 = v_s.lengthSquare();
    v_s.normalize();
    Vector drag_force_s = - v_s * (cw_s * v2 * side_area * rho / 2.0);

    applyForce(drag_force_f + drag_force_s);
    applyLinearAcceleration(Vector(0,-9.81,0));

    RigidEngine::run();
}
*/

void MissileEngine2::setControls(Ptr<Controls> controls) {
}

void MissileEngine2::run() {
    // air density
    const static float rho = 1.293;

    Vector d = getFrontVector();
    Vector v = getMovementVector();

    // stabilizing torque
    Vector omega = getAngularVelocity();
    Vector omega_z = d * (omega*d);
    Vector omega_xy = omega - omega_z;
    Vector v_z = d * (v*d);
    Vector v_xy = v - v_z;
    applyTorque(-300 * rho * omega_z.length()*omega_z);
    applyTorque(-100 * rho * omega_xy.length()*omega_xy);

    // frontal drag coefficient
    const static float cw_f = 0.3;
    Vector v_f = d * (v*d); // frontal component of velocity
    Vector drag_force_f = -v_f.length() * v_f * (cw_f * front_area * rho / 2.0);

    // side drag coefficient
    const static float cw_s = 1.2;
    Vector v_s = v - v_f; // side component of velocity
    Vector drag_force_s = -v_s.length() * v_s * (cw_s * side_area * rho / 2.0);

    applyForce(drag_force_f + drag_force_s);
    applyLinearAcceleration(Vector(0,-9.81,0));

    RigidEngine::run();
}
