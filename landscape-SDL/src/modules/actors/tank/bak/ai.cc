#include <modules/math/Ballistic.h>
#include <modules/math/Feedback.h>
#include "ai.h"

/////////////////////////////////////////////////////////////////////////////
// class CruiseControl
// tries to reach and maintain a certain speed
/////////////////////////////////////////////////////////////////////////////

CruiseControl::CruiseControl(Ptr<Clock> clock,
                             Ptr<IActor> car,
                             Ptr<CarControls> controls)
:   AI::Idea("CruiseControl"),
    clock(clock), car(car), controls(controls), error(0.0f), last_error(0.0f),
    target_v(0.0f)
{ }

void CruiseControl::setTargetSpeed(float v) {
    target_v = v;
}

void CruiseControl::onEnabled() {
    calcError();
}

void CruiseControl::onDisabled() { }

void CruiseControl::onThink() {
    last_error = error;
    calcError();
    
    float delta_t = clock->getStepDelta();
    float delta_error = error - last_error;
    float derror = delta_error / delta_t;
    
    float control = controls->getThrottle() - controls->getBrake();
    
    FeedbackController<float> ctlr(0.0f, -0.3f, +0.1f);
    float delta_control = ctlr.control(error, derror) * delta_t;
    control += delta_control;
    
    if (control > 1.0f) control = 1.0f;
    else if (control < -1.0f) control = -1.0f;
    
    controls->setThrottle(std::max(0.0f, control));
    controls->setBrake(std::max(0.0f, -control));
}

void CruiseControl::calcError() {
    float v = car->getMovementVector() * car->getFrontVector();
    error = target_v - v;
    
    char buf[256];
    sprintf(buf, "CruiseControl:\n"
                 "Current velocity: %3.1f m/s\n"
                 "Target velocity:  %3.1f m/s\n"
                 "Current throttle: %2.1f%\n"
                 "Current brake:    %2.1f%\n",
            v, target_v,
            controls->getThrottle()*100.0f,
            controls->getBrake()*100.0f);
    info = buf;
}


/////////////////////////////////////////////////////////////////////////////
// class CourseControl
// tries to reach and maintain a certain course
/////////////////////////////////////////////////////////////////////////////

CourseControl::CourseControl(Ptr<Clock> clock,
                             Ptr<IActor> car,
                             Ptr<CarControls> controls)
:   AI::Idea("CourseControl"),
    clock(clock), car(car), controls(controls), error(0.0f), last_error(0.0f),
    target_course(0.0f, 0.0f, 1.0f)
{ }

void CourseControl::setTargetCourse(const Vector & course) {
    target_course = course;
}

void CourseControl::onEnabled() {
    calcError();
}

void CourseControl::onDisabled() { }

void CourseControl::onThink() {
    last_error = error;
    calcError();
    
    float delta_t = clock->getStepDelta();
    float delta_error = error - last_error;
    float derror = delta_error / delta_t;
    
    FeedbackController<float> ctlr(0.0f, -1.5f, 0.1f);
    float control = ctlr.control(error, derror);
    
    if (car->getMovementVector() * car->getFrontVector() < 0.0f) {
        control = -control;
    }
    
    if (control > 1.0f) control = 1.0f;
    else if (control < -1.0f) control = -1.0f;
    
    controls->setSteer(control);
}

void CourseControl::calcError() {
    error = car->getRightVector() * target_course;
    if (car->getFrontVector() * target_course < 0.0f) {
        if (error >= 0.0f) error = 1.0f;
        else error = -1.0f;
    }
    
    char buf[256];
    sprintf(buf, "CourseControl:\n"
                 "Current course error: %+1.2f\n"
                 "Current steer:        %+2.1f%\n",
            error,
            controls->getSteer()*100.0f);
    info = buf;
}

/////////////////////////////////////////////////////////////////////////////
// class CannonControl
// tries to reach and maintain a certain cannon vector
/////////////////////////////////////////////////////////////////////////////

CannonControl::CannonControl(Ptr<Clock> clock, Ptr<TankEngine> tank_engine,
        Ptr<TankControls> controls)
:   AI::Idea("CannonControl"),
    clock(clock), tank_engine(tank_engine), controls(controls)
{  }

void CannonControl::setTargetVector(const Vector & v) {
    target_vector = v;
}

void CannonControl::onEnabled() {
    calcError();
}

void CannonControl::onDisabled() { }

void CannonControl::onThink() {
    last_cannon_error = cannon_error;
    last_turret_error = turret_error;
    calcError();
    
    float delta_t = clock->getStepDelta();
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
    
    controls->setTurretSteer(turret_control);
    controls->setCannonSteer(cannon_control);
}


void CannonControl::calcError() {
    Vector v = tank_engine->getRelativeCannonVector();
    Vector up(0,1,0);
    Vector right = (up % v).normalize();
    up = v % right;
    cannon_error = target_vector * up;
    turret_error = target_vector * right;
    if ( target_vector * v < 0.0f) {
        if (turret_error >= 0.0f) turret_error = 1.0f;
        else turret_error = -1.0f;
    }
    
    char buf[256];
    sprintf(buf, "CannonControl:\n"
                 "Target vector:  %+1.2f %+1.2f %+1.2f\n"
                 "Current vector: %+1.2f %+1.2f %+1.2f\n"
                 "Turret error:   %+1.2f\n"
                 "Turret steer:   %+2.1f%\n"
                 "Cannon error:   %+1.2f\n"
                 "Cannon steer:   %+2.1f%\n",
            target_vector[0], target_vector[1], target_vector[2],
            v[0], v[1], v[2],
            turret_error, controls->getTurretSteer()*100.0f,
            cannon_error, controls->getCannonSteer()*100.0f);
    info = buf;
}


/////////////////////////////////////////////////////////////////////////////
// class AbsoluteCannonControl
// tries to reach and maintain a certain cannon vector in world coordinates
/////////////////////////////////////////////////////////////////////////////

AbsoluteCannonControl::AbsoluteCannonControl(
        Ptr<Clock> clock,
        Ptr<TankEngine> tank_engine,
        Ptr<TankControls> controls,
        Ptr<CannonControl> cannon_control)
:   AI::Idea("AbsoluteCannonControl"),
    tank_engine(tank_engine),
    target_vector(0,0,1),
    cannon_control(cannon_control)
{ }


void AbsoluteCannonControl::setTargetVector(const Vector & v) {
    target_vector = v;
}

void AbsoluteCannonControl::onEnabled() { cannon_control->setEnabled(true); }
void AbsoluteCannonControl::onDisabled() { cannon_control->setEnabled(false); }
void AbsoluteCannonControl::onThink() {
    Vector right, up, front;
    tank_engine->getOrientation(&up, &right, &front);
    cannon_control->setTargetVector( Vector(
            target_vector * right,
            target_vector * up,
            target_vector * front));
    cannon_control->think();
    info = cannon_control->getInfo();
}


/////////////////////////////////////////////////////////////////////////////
// class BallisticCannonControl
// calculates a firing solution for a given target
/////////////////////////////////////////////////////////////////////////////

BallisticCannonControl::BallisticCannonControl(Ptr<IActor> source,
        Ptr<AbsoluteCannonControl> cannon_control)
:   AI::Idea("BallisticCannonControl"),
    source(source),
    cannon_control(cannon_control),
    muzzle_vel(900.0f),
    target(0,0,1000)
{ }


void BallisticCannonControl::setTarget(const Vector & target) {
    this->target = target;
}

void BallisticCannonControl::setMuzzleVelocity(float mv) {
    muzzle_vel = mv;
}

void BallisticCannonControl::onEnabled() { cannon_control->setEnabled(true); }
void BallisticCannonControl::onDisabled() { cannon_control->setEnabled(false); }
void BallisticCannonControl::onThink() {
    Vector d = target - source->getLocation();
    Vector right = Vector(d[0], 0.0f, d[2]);
    right.normalize();
    
    float dx = d*right;
    float dy = d[1];
    float v0 = muzzle_vel;
    float g = -9.81f;
    float vx0, vy0, vx1, vy1;
    
    if (!Ballistic::solve(dx, dy, v0, g, &vx0, &vy0, &vx1, &vy1)) {
        info = "No firing solution found.";
        cannon_control->setEnabled(false);
        return;
    }
    
    Vector solution = right * vx1 + Vector(0,1,0) * vy1;
    solution.normalize();
    cannon_control->setEnabled(true);
    cannon_control->setTargetVector(solution);
    cannon_control->think();
}

/////////////////////////////////////////////////////////////////////////////
// class MaintainPosition
// tries to maintain a given position at a given speed
/////////////////////////////////////////////////////////////////////////////

MaintainPosition::MaintainPosition(Ptr<IActor> source,
        Ptr<CourseControl> course_control,
        Ptr<CruiseControl> cruise_control,
        Ptr<CarControls> controls)
:   AI::Idea("MaintainPosition"),
    source(source),
    course_control(course_control),
    cruise_control(cruise_control),
    controls(controls),
    approach_speed(16.0f),
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

void MaintainPosition::onEnabled() {
    cruise_control->setEnabled(true);
}

void MaintainPosition::onDisabled() {
    course_control->setEnabled(false);
    cruise_control->setEnabled(false);
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

void MaintainPosition::onThink() {
    Vector p = source->getLocation();
    Vector v = source->getMovementVector();
    Vector front = source->getFrontVector();
    Vector right = source->getRightVector();

    p[1] = 0;
    v[1] = 0;
    front[1] = 0;
    right[1] = 0;
    front = norm(front, Vector(0,0,1));
    right = norm(right, Vector(1,0,0));

    Vector delta_p = target_p - p;
    Vector delta_v = target_v - v;

    char buf[256];
    sprintf(buf, "Maintaining Position:\n"
                 "delta_p : %+5.2f %+5.2f\n"
                 "delta_v : %+5.2f %+5.2f\n",
                 delta_p[0], delta_p[2], delta_v[0], delta_v[2]);

    course_control->setEnabled(true);
    /*
    float d_lat = delta_p * right;
    float d_lon = delta_p * front;
    float speed_diff = approach_speed - target_v * front;
    float ideal_speed = target_v * front - (d_lon / 20.0f) * speed_diff;
    ideal_speed += ( std::abs(d_lat) / 20.0f ) * speed_diff;
    ideal_speed = std::max(0.0f, std::min(approach_speed, ideal_speed));
    cruise_control->setTargetSpeed(ideal_speed); */

    float dist = delta_p.length();
    float ideal_speed;
    float max_r = 80.0f, min_r = 5.0f;
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
    course_control->setTargetCourse(ideal_course);
    cruise_control->setTargetSpeed(ideal_speed);

    cruise_control->think();
    course_control->think();

    sprintf(buf, "%s\n"
                "Pursuing\n"
                "ideal_course: %+5.2f %+5.2f\n"
                "ideal_speed: %3.2f\n"
                "\n%s",
            buf, ideal_course[0],ideal_course[2], ideal_speed,
            cruise_control->getInfo().c_str());
    info = buf;
}
