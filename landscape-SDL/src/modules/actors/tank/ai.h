#ifndef TANK_AI_H
#define TANK_AI_H

#include <landscape.h>
#include <modules/ai/Idea.h>
#include <modules/engines/controls.h>
#include <modules/clock/clock.h>
#include <modules/math/Rendezvous.h>
#include "tank.h"


struct TankBrain;

class TankIdea;
class TankIdea : public AI::IdeaBase<TankBrain, TankIdea>
{
protected:
    virtual void onAimingConeEntered()
    { if(parent) parent->onAimingConeEntered(); }
    virtual void onAimingConeLeft()
    { if(parent) parent->onAimingConeLeft(); }
};

class CruiseControl : public TankIdea {
public:
    CruiseControl(TankBrain & brain);
    void setTargetSpeed(float v);

    virtual void onEnabled(TankBrain & brain);
    virtual void act(TankBrain & brain);

private:
    void calcError(TankBrain & brain);

    float error, last_error;
    float target_v;
};

class CourseControl : public TankIdea {
public:
    CourseControl(TankBrain & brain);
    void setTargetCourse(const Vector & course);

    virtual void onEnabled(TankBrain & brain);
    virtual void act(TankBrain & brain);

protected:
    void calcError(TankBrain & brain);

private:
    float error, last_error;
    Vector target_course;
};

class CannonControl : public TankIdea {
    static const float epsilon;
public:
    CannonControl(TankBrain & brain);
    void setTargetVector(const Vector & v);

    virtual void onEnabled(TankBrain & brain);
    virtual void onDisabled(TankBrain & brain);
    virtual void act(TankBrain & brain);
    inline bool inAimingCone() { return error <= epsilon; }

private:
    void calcError(TankBrain & brain);

private:
    float error;
    float cannon_error, last_cannon_error;
    float turret_error, last_turret_error;
    Vector target_vector;
};

class AbsoluteCannonControl : public TankIdea {
public:
    AbsoluteCannonControl(TankBrain & brain);
    void setTargetVector(const Vector & v);

    virtual void think(TankBrain & brain);

private:
    Vector target_vector;
};

class BallisticCannonControl : public TankIdea {
public:
    BallisticCannonControl(TankBrain & brain);
    void setTarget(const Vector & target);
    void setMuzzleVelocity(float mv);

    virtual void think(TankBrain & brain);

private:
    float muzzle_vel;
    Vector target;
};

class AdvancedCannonControl : public TankIdea {
public:
    AdvancedCannonControl(TankBrain & brain);
    void setTarget(const Vector & target);
    void setMuzzleVelocity(float mv);

    virtual void think(TankBrain & brain);

private:
    float muzzle_vel;
    Vector target;
    Rendezvous rendezvous;
};


class MaintainPosition : public TankIdea {
public:
    MaintainPosition(TankBrain & brain);
    void setTarget(const Vector & p, const Vector & v);
    void setApproachSpeed(float s);

    virtual void think(TankBrain & brain);

private:
    Vector target_p, target_v;
    float approach_speed;
};


class MoveToExposedPoint : public TankIdea {
public:
    virtual void think(TankBrain & brain);
    virtual void onEnabled(TankBrain & brain);
private:
    Vector exposed_point;
};
    

struct TankBrain : public Object {
    inline TankBrain(Ptr<IGame>         game,
                     Ptr<Clock>         clock,
                     Ptr<IActor>        tank,
                     Ptr<TankControls>  controls,
                     Ptr<TankEngine>    engine)
    :   game(game),
        clock(clock),
        tank(tank),
        controls(controls),
        engine(engine),

        cruise_control(*this),
        course_control(*this),
        cannon_control(*this),
        absolute_cannon_control(*this),
        ballistic_cannon_control(*this),
        advanced_cannon_control(*this),
        maintain_position(*this)
    { }

    Ptr<IGame>                  game;
    Ptr<Clock>                  clock;
    Ptr<IActor>                 tank;
    Ptr<TankControls>           controls;
    Ptr<TankEngine>             engine;

    CruiseControl               cruise_control;
    CourseControl               course_control;
    CannonControl               cannon_control;
    AbsoluteCannonControl       absolute_cannon_control;
    BallisticCannonControl      ballistic_cannon_control;
    AdvancedCannonControl       advanced_cannon_control;
    MaintainPosition            maintain_position;
    MoveToExposedPoint          move_to_exposed_point;
};


#endif
