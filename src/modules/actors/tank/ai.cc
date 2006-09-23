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
:   error(0.0f), last_error(0.0f), target_course(0.0f, 1.0f)
{ }

void CourseControl::setTargetCourse(const Vector2 & course) {
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
	Vector2 right(
		brain.tank->getRightVector()[0],
		brain.tank->getRightVector()[2]);
	Vector2 front(
		brain.tank->getFrontVector()[0],
		brain.tank->getFrontVector()[2]);
    error = right * target_course;
    if (front * target_course < 0.0f) {
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
:   muzzle_vel(900.0f), target(0,0,1000)
{
    rendezvous.setVelocity(muzzle_vel);
}

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
    target_p(0,0), target_v(0,0)
{ }

void MaintainPosition::setTarget(const Vector & p, const Vector & v) {
    target_p = Vector2(p[0],p[2]);
    target_v = Vector2(v[0],v[2]);
}

void MaintainPosition::setApproachSpeed(float s) {
    approach_speed = s;
}


namespace {
    //inline float normalize(float t, float min, float max) { return (t-min)/(max-min); }
    //inline float linterp(float t, float a, float b) { return a + t * (b-a); }
    inline Vector2 linterp(float t, Vector2 a, Vector2 b) { return a + t * (b-a); }
    inline Vector2 norm(const Vector2 & v, const Vector2 & def) {
        float v2 = v.lengthSquare();
        if (v2 > 0) return v / sqrt(v2);
        else return def;
    }
}

void MaintainPosition::think(TankBrain & brain) {
    Vector2 p(
    	brain.tank->getLocation()[0],
    	brain.tank->getLocation()[2]);
    	
    // The minimum distance at which an intercept course
    // is chosen. Below that distance, we will gradually
    // choose a parallel course.
    float min_intercept_radius=50;
    float tolerance_radius=2;
    
    float dist2target = (target_p - p).length();
    Vector2 dir2target = norm(target_p - p, Vector2(1,0));
    // target velocity parallel
    Vector2 tvpa = dir2target * (dir2target * target_v);
    // target direction parallel
    Vector2 tdpa = norm(tvpa, Vector2(1,0));
    // target velocity perpendicular
    Vector2 tvpe = target_v - tvpa;
    // target direction perpendicular
    Vector2 tdpe = norm(tvpe, Vector2(tdpa[0],-tdpa[1]));

    if (tvpe.lengthSquare() >= approach_speed*approach_speed) {
    	
	    brain.course_control.setTargetCourse(tdpe);
	    brain.cruise_control.setTargetSpeed(approach_speed);
	    activate(brain.cruise_control);
	    activate(brain.course_control);
	    return;
    }
    	
   	Vector2 intercept_v =
   		tvpe+dir2target*sqrt(
   			approach_speed*approach_speed
   			- tvpe.lengthSquare());
    Vector2 desired_v;
    
    if (dist2target >= min_intercept_radius) {
    	desired_v = intercept_v;
    } else if ( dist2target >= tolerance_radius) {
    	float t = (dist2target-tolerance_radius)
    		/ (min_intercept_radius-tolerance_radius);
    	desired_v = linterp(t, tvpa, intercept_v);
    } else {
    	desired_v = tvpa;
    }
    
    char buf[1024];
    snprintf(buf, 1024, "Tank at % 7.1f, % 7.1f, target at % 7.1f, % 7.1f\n",
    	p[0],p[1], target_p[0], target_p[1]);
    brain.info += buf;
    snprintf(buf, 1024, "      v % 7.1f, % 7.1f,         v % 7.1f, % 7.1f\n",
    	brain.tank->getMovementVector()[0],
    	brain.tank->getMovementVector()[2],
    	target_v[0], target_v[1]);
    brain.info += buf;
    snprintf(buf, 1024, "   --> dir2target: % 7.4f, % 7.4f [%7.1f]\n",
    	dir2target[0], dir2target[1], dist2target);
    brain.info += buf;
    snprintf(buf, 1024, "   --> tvpa: % 7.1f, % 7.1f\n",
    	tvpa[0], tvpa[1]);
    brain.info += buf;
    snprintf(buf, 1024, "   --> tvpe: % 7.1f, % 7.1f\n",
    	tvpe[0], tvpe[1]);
    brain.info += buf;
    snprintf(buf, 1024, "   --> intercept: % 7.1f, % 7.1f\n",
    	intercept_v[0], intercept_v[1]);
    brain.info += buf;
    snprintf(buf, 1024, "   --> course: % 7.1f, % 7.1f\n",
    	desired_v[0], desired_v[1]);
    brain.info += buf;
    
    brain.cruise_control.setTargetSpeed(desired_v.length());
    brain.course_control.setTargetCourse(
    	norm(desired_v, norm(target_v, dir2target)));

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
    //ls_error("exposed point: "); exposed_point.dump();
}


        
                
