#ifndef TANK_AI_H
#define TANK_AI_H

#include <landscape.h>
#include <modules/ai/Idea.h>
#include <modules/engines/controls.h>
#include <modules/clock/clock.h>
#include "tank.h"


class TankBrain;
class TankIdea;
class TankIdea : public AI::IdeaBase<TankBrain, TankIdea>
{
};


class CruiseControl : public AI::Idea {
public:
    CruiseControl(Ptr<Clock> clock, Ptr<IActor> car, Ptr<CarControls> controls);
    void setTargetSpeed(float v);
    
protected:
    virtual void onEnabled();
    virtual void onDisabled();
    virtual void onThink();
    void calcError();
    
private:
    Ptr<Clock> clock;
    Ptr<IActor> car;
    Ptr<CarControls> controls;
    float error, last_error;
    float target_v;
};

class CourseControl : public AI::Idea {
public:
    CourseControl(Ptr<Clock> clock, Ptr<IActor> car, Ptr<CarControls> controls);
    void setTargetCourse(const Vector & course);

protected:
    virtual void onEnabled();
    virtual void onDisabled();
    virtual void onThink();
    void calcError();
    
private:
    Ptr<Clock> clock;
    Ptr<IActor> car;
    Ptr<CarControls> controls;
    float error, last_error;
    Vector target_course;
};

class CannonControl : public AI::Idea {
public:
    CannonControl(Ptr<Clock> clock, Ptr<TankEngine> tank_engine,
        Ptr<TankControls> controls);
    void setTargetVector(const Vector & v);
    
protected:
    virtual void onEnabled();
    virtual void onDisabled();
    virtual void onThink();
    void calcError();
    
private:
    Ptr<Clock> clock;
    Ptr<TankEngine> tank_engine;
    Ptr<TankControls> controls;
    float cannon_error, last_cannon_error;
    float turret_error, last_turret_error;
    Vector target_vector;
};

class AbsoluteCannonControl : public AI::Idea {
public:
    AbsoluteCannonControl(Ptr<Clock> clock, Ptr<TankEngine> tank_engine,
        Ptr<TankControls> controls, Ptr<CannonControl> cannon_control);
    void setTargetVector(const Vector & v);
    
protected:
    virtual void onEnabled();
    virtual void onDisabled();
    virtual void onThink();
    
private:
    Vector target_vector;
    Ptr<TankEngine> tank_engine;
    Ptr<CannonControl> cannon_control;
};

class BallisticCannonControl : public AI::Idea {
public:
    BallisticCannonControl(Ptr<IActor> source, Ptr<AbsoluteCannonControl> cannon_control);
    void setTarget(const Vector & target);
    void setMuzzleVelocity(float mv);
    
protected:
    virtual void onEnabled();
    virtual void onDisabled();
    virtual void onThink();
    
private:
    float muzzle_vel;
    Ptr<IActor> source;
    Vector target;
    Ptr<AbsoluteCannonControl> cannon_control;
};

class MaintainPosition : public AI::Idea {
public:
    MaintainPosition(Ptr<IActor> source,
        Ptr<CourseControl> course_control,
        Ptr<CruiseControl> cruise_control,
        Ptr<CarControls> controls);
    void setTarget(const Vector & p, const Vector & v);
    void setApproachSpeed(float s);

protected:
    virtual void onEnabled();
    virtual void onDisabled();
    virtual void onThink();
    
private:
    Vector target_p, target_v;
    float approach_speed;
    Ptr<IActor> source;
    Ptr<CourseControl> course_control;
    Ptr<CruiseControl> cruise_control;
    Ptr<CarControls> controls;
};

#endif
