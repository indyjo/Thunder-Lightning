#include <modules/math/Ballistic.h>
#include <modules/math/Feedback.h>
#include "ai.h"
#include <interfaces/ITerrain.h>


/////////////////////////////////////////////////////////////////////////////
// class CruiseControl
// tries to reach and maintain a certain speed
/////////////////////////////////////////////////////////////////////////////

CruiseControl::CruiseControl(TankBrain & brain)
:   error(0.0f), last_error(0.0f),target_v(0.0f)
{ }

void CruiseControl::setTargetSpeed(float v) {
    target_v = v;
}

void CruiseControl::onEnabled(TankBrain & brain) {
    calcError(brain);
}

void CruiseControl::act(TankBrain & brain) {
    last_error = error;
    calcError(brain);

    float delta_t = brain.clock->getStepDelta();
    float delta_error = error - last_error;
    float derror = delta_error / delta_t;

    float control = brain.controls->getThrottle() - brain.controls->getBrake();

    FeedbackController<float> ctlr(0.0f, -0.3f, +0.1f);
    float delta_control = ctlr.control(error, derror) * delta_t;
    control += delta_control;

    if (control > 1.0f) control = 1.0f;
    else if (control < -1.0f) control = -1.0f;

    brain.controls->setThrottle(std::max(0.0f, control));
    brain.controls->setBrake(std::max(0.0f, -control));
}

void CruiseControl::calcError(TankBrain & brain) {
    float v = brain.tank->getMovementVector() * brain.tank->getFrontVector();
    error = target_v - v;
}


/////////////////////////////////////////////////////////////////////////////
// class CourseControl
// tries to reach and maintain a certain course
/////////////////////////////////////////////////////////////////////////////

CourseControl::CourseControl(TankBrain & brain)
:   error(0.0f), last_error(0.0f), target_course(0.0f, 0.0f, 1.0f)
{ }

void CourseControl::setTargetCourse(const Vector & course) {
    target_course = course;
}

void CourseControl::onEnabled(TankBrain & brain) {
    calcError(brain);
}

void CourseControl::act(TankBrain & brain) {
    last_error = error;
    calcError(brain);

    float delta_t = brain.clock->getStepDelta();
    float delta_error = error - last_error;
    float derror = delta_error / delta_t;

    FeedbackController<float> ctlr(0.0f, -1.5f, 0.1f);
    float control = ctlr.control(error, derror);

    if (brain.tank->getMovementVector() * brain.tank->getFrontVector() < 0.0f) {
        control = -control;
    }

    if (control > 1.0f) control = 1.0f;
    else if (control < -1.0f) control = -1.0f;

    brain.controls->setSteer(control);
}

void CourseControl::calcError(TankBrain & brain) {
    error = brain.tank->getRightVector() * target_course;
    if (brain.tank->getFrontVector() * target_course < 0.0f) {
        if (error >= 0.0f) error = 1.0f;
        else error = -1.0f;
    }
}

/////////////////////////////////////////////////////////////////////////////
// class CannonControl
// tries to reach and maintain a certain cannon vector
/////////////////////////////////////////////////////////////////////////////

const float CannonControl::epsilon = 5.5e-3;

CannonControl::CannonControl(TankBrain & brain)
:   error(0.0f)
{ }

void CannonControl::setTargetVector(const Vector & v) {
    target_vector = v;
}

void CannonControl::onEnabled(TankBrain & brain) {
    calcError(brain);
}

void CannonControl::onDisabled(TankBrain & brain) { }

void CannonControl::act(TankBrain & brain) {
    if (target_vector.lengthSquare() < 0.0001) return;
    last_cannon_error = cannon_error;
    last_turret_error = turret_error;
    calcError(brain);

    float delta_t = brain.clock->getStepDelta();
    float delta_cannon_error = cannon_error - last_cannon_error;
    float dcannon_error = delta_cannon_error / delta_t;
    float delta_turret_error = turret_error - last_turret_error;
    float dturret_error = delta_turret_error / delta_t;

    FeedbackController<float> ctlr(0.0f, -5.0f, 0.01f);
    float turret_control = ctlr.control(turret_error, dturret_error);
    float cannon_control = ctlr.control(cannon_error, dcannon_error);

    if (turret_control > 1.0f) turret_control = 1.0f;
    else if (turret_control < -1.0f) turret_control = -1.0f;
    if (cannon_control > 1.0f) cannon_control = 1.0f;
    else if (cannon_control < -1.0f) cannon_control = -1.0f;

    brain.controls->setTurretSteer(turret_control);
    brain.controls->setCannonSteer(cannon_control);
}


void CannonControl::calcError(TankBrain & brain) {
    Vector v = brain.engine->getRelativeCannonVector();
    Vector up(0,1,0);
    Vector right = (up % v).normalize();
    up = v % right;
    up.normalize();
    cannon_error = target_vector * up;
    turret_error = target_vector * right;
    if ( target_vector * v < 0.0f) {
        if (turret_error >= 0.0f) turret_error = 1.0f;
        else turret_error = -1.0f;
    }
    float old_error = error;
    error = cannon_error * cannon_error + turret_error * turret_error;
    if (old_error > epsilon && error <= epsilon)
        onAimingConeEntered();
    else if (old_error <= epsilon && error > epsilon)
        onAimingConeLeft();
}


/////////////////////////////////////////////////////////////////////////////
// class AbsoluteCannonControl
// tries to reach and maintain a certain cannon vector in world coordinates
/////////////////////////////////////////////////////////////////////////////

AbsoluteCannonControl::AbsoluteCannonControl(TankBrain & brain)
:   target_vector(0,0,1)
{ }


void AbsoluteCannonControl::setTargetVector(const Vector & v) {
    target_vector = v;
}

void AbsoluteCannonControl::think(TankBrain & brain) {
    Vector right, up, front;
    brain.engine->getOrientation(&up, &right, &front);
    brain.cannon_control.setTargetVector( Vector(
            target_vector * right,
            target_vector * up,
            target_vector * front));
    activate(brain.cannon_control);
}


/////////////////////////////////////////////////////////////////////////////
// class BallisticCannonControl
// calculates a firing solution for a given target
/////////////////////////////////////////////////////////////////////////////

BallisticCannonControl::BallisticCannonControl(TankBrain & brain)
:   muzzle_vel(900.0f),
    target(0,0,1000)
{ }


void BallisticCannonControl::setTarget(const Vector & target) {
    this->target = target;
}

void BallisticCannonControl::setMuzzleVelocity(float mv) {
    muzzle_vel = mv;
}

void BallisticCannonControl::think(TankBrain & brain) {
    Vector d = target - brain.tank->getLocation();
    Vector right = Vector(d[0], 0.0f, d[2]);
    right.normalize();

    float dx = d*right;
    float dy = d[1];
    float v0 = muzzle_vel;
    float g = -9.81f;
    float vx0, vy0, vx1, vy1;

    if (!Ballistic::solve(dx, dy, v0, g, &vx0, &vy0, &vx1, &vy1)) {
        return;
    }

    Vector solution = right * vx1 + Vector(0,1,0) * vy1;
    solution.normalize();
    brain.absolute_cannon_control.setTargetVector(solution);
    activate(brain.absolute_cannon_control);
}

/////////////////////////////////////////////////////////////////////////////
// class AdvancedCannonControl
// calculates a firing solution for a given target considering its current
// position, velocity and acceleration
/////////////////////////////////////////////////////////////////////////////

AdvancedCannonControl::AdvancedCannonControl(TankBrain & brain)
:   muzzle_vel(900.0f),
    target(0,0,1000)
{ }


void AdvancedCannonControl::setTarget(const Vector & target) {
    this->target = target;
}

void AdvancedCannonControl::setMuzzleVelocity(float mv) {
    muzzle_vel = mv;
}

void AdvancedCannonControl::think(TankBrain & brain) {
    Vector source = brain.tank->getLocation();
    if ((source-target).lengthSquare() < 0.0001f) return;
    rendezvous.updateSource(
        brain.clock->getStepDelta(),
        source,
        (target-source).normalize()*muzzle_vel);
    rendezvous.updateTarget(brain.clock->getStepDelta(), target);
    brain.ballistic_cannon_control.setTarget(rendezvous.calculate());
    brain.ballistic_cannon_control.setMuzzleVelocity(muzzle_vel);
    activate(brain.ballistic_cannon_control);
}


/////////////////////////////////////////////////////////////////////////////
// class MaintainPosition
// tries to maintain a given position at a given speed
/////////////////////////////////////////////////////////////////////////////

MaintainPosition::MaintainPosition(TankBrain & brain)
:   approach_speed(16.0f),
    target_p(0,0,0), target_v(0,0,0)
{ }

void MaintainPosition::setTarget(const Vector & p, const Vector & v) {
    target_p = p;
    target_v = v;
    target_p[1] = 0;
    target_v[1] = 0;
}

void MaintainPosition::setApproachSpeed(float s) {
    approach_speed = s;
}


namespace {
    inline float normalize(float t, float min, float max) { return (t-min)/(max-min); }
    inline float linterp(float t, float a, float b) { return a + t * (b-a); }
    inline Vector linterp(float t, Vector a, Vector b) { return a + t * (b-a); }
    inline Vector norm(const Vector & v, const Vector & def) {
        float v2 = v.lengthSquare();
        if (v2 > 0) return v / sqrt(v2);
        else return def;
    }
}

void MaintainPosition::think(TankBrain & brain) {
    Vector p = brain.tank->getLocation();
    Vector v = brain.tank->getMovementVector();
    Vector front = brain.tank->getFrontVector();
    Vector right = brain.tank->getRightVector();

    p[1] = 0;
    v[1] = 0;
    front[1] = 0;
    right[1] = 0;
    front = norm(front, Vector(0,0,1));
    right = norm(right, Vector(1,0,0));

    Vector delta_p = target_p - p;
    Vector delta_v = target_v - v;

    float dist = delta_p.length();
    float ideal_speed;
    float max_r = 80.0f, min_r = 15.0f;
    Vector ideal_course;
    if (dist > max_r) {
        ideal_speed = approach_speed;
        ideal_course = delta_p;
    } else if (dist > min_r) {
        float t = normalize(dist, min_r, max_r);
        ideal_speed = linterp(t, target_v.length(), approach_speed);
        Vector target_course = norm(target_v, front);
        ideal_course = linterp(t, target_course, front);
    } else {
        ideal_speed = target_v.length();
        ideal_course = norm(target_v, front);
    }


    //Vector ideal_course = front - (d_lat / 30.0f) * right;
    //ideal_course = delta_p;
    ideal_course = norm(ideal_course, front);
    brain.course_control.setTargetCourse(ideal_course);
    brain.cruise_control.setTargetSpeed(ideal_speed);

    activate(brain.cruise_control);
    activate(brain.course_control);
}


/////////////////////////////////////////////////////////////////////////////
// class MoveToExposedPoint
// finds the highest point in the environment and moves towards that point
/////////////////////////////////////////////////////////////////////////////

void MoveToExposedPoint::think(TankBrain & brain) {
    brain.maintain_position.setTarget(exposed_point, Vector(0,0,0));
    activate(brain.maintain_position);
}

#define PI 3.141593f
#define NCANDIDATES 5
#define CANDIDATE_DISTANCE 20.0f

void MoveToExposedPoint::onEnabled(TankBrain & brain) {
    Vector best_point = brain.tank->getLocation();
    Ptr<ITerrain> terrain = brain.game->getTerrain();
    best_point[1] = terrain->getHeightAt(best_point[0],best_point[2]);
    Vector best_candidate_point = best_point;
    
    do {
        best_point = best_candidate_point;
        Vector candidate_point;
        for(int i=0; i<NCANDIDATES; ++i) {
            candidate_point[0] = best_candidate_point[0] +
                cos(2*PI*i/NCANDIDATES) * CANDIDATE_DISTANCE;
            candidate_point[2] = best_candidate_point[2] +
                sin(2*PI*i/NCANDIDATES) * CANDIDATE_DISTANCE;
            candidate_point[1] = terrain->getHeightAt(
                candidate_point[0], candidate_point[2]);
            if (candidate_point[1] > best_candidate_point[1])
                best_candidate_point = candidate_point;
        }
    } while (best_candidate_point[1] > best_point[1]);
    
    exposed_point = best_point;
    ls_error("exposed point: "); exposed_point.dump();
}


        
                
