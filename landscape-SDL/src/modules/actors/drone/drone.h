#ifndef DRONE_H
#define DRONE_H

#include <landscape.h>
#include <mtasker.h>
#include <modules/math/Vector.h>
#include <modules/math/Matrix.h>
#include <modules/model/model.h>
#include <modules/flight/autopilot.h>
#include <modules/flight/flightinfo.h>
#include <modules/engines/controls.h>
#include <modules/actors/simpleactor.h>
#include <modules/collide/CollisionManager.h>

class FlightEngine2;
class Idea;
class PatrolIdea;
struct Rating;
class Targeter;
struct SoundSource;
struct Context;

class EventSheet;

struct Personality {
    Personality() : confidence(.25), obedience(.25),
            experience(.25), cautiousness(.25) { }
    float confidence;
    float obedience;
    float experience;
    float cautiousness;
    
    float evaluate(const Rating & r);
    void randomize();
};

struct Wheel {
	Vector pos;
	float range, force, damping, drag_long, drag_lat;
};

struct WheelState {
	bool contact;
	Vector pos;
	float pressure;
};

class Drone : public SimpleActor, public Collide::Collidable, virtual public SigObject {
public:
    Drone(Ptr<IGame> thegame);

    virtual void action();
    virtual void kill();

    virtual void draw();

    virtual void applyDamage(float damage, int domain);
    virtual float getRelativeDamage();

    virtual void integrate(float delta_t, Transform * transforms);
    virtual void update(float delta_t, const Transform * new_transforms);
    
    virtual int getNumViews();
    virtual Ptr<IView> getView(int n);

    void explode();

    void fireBullet();
    void fireDumbMissile();
    void fireSmartMissile();
    void fireSmartMissile2();
    
    virtual bool hasControlMode(ControlMode);
    virtual void setControlMode(ControlMode);
    
private:
    enum Event {
        CYCLE_PRIMARY,
        CYCLE_SECONDARY,
        FIRE_PRIMARY,
        RELEASE_PRIMARY,
        FIRE_SECONDARY,
        RELEASE_SECONDARY
    };
    void event(Event e);
    void doWheels();
    void drawWheels();
    
private:
    JRenderer * renderer;
    Ptr<ITerrain> terrain;
    Ptr<SoundSource> engine_sound_src;

    // ai stuff
    Ptr<Context> context;
    Ptr<EventSheet> event_sheet;
    ControlMode control_mode;
    Personality personality;
    std::list<Ptr<Idea> > ideas;
    Ptr<Idea> current_idea;
    Ptr<PatrolIdea> patrol_idea;

    // 3d model
    Ptr<Model> outside_model, inside_model;
    Ptr<Model> wheel_model;

    // flight stuff
    Ptr<FlightEngine2> engine;
    FlightInfo flight_info;
    Ptr<DroneControls> drone_controls;
    AutoPilot auto_pilot;
    WheelState wheel_states[3];

    // weapon systems stuff
    Ptr<Targeter> targeter;
    int cannon_num, smart_launcher_num, dumb_launcher_num;
    float primary_reload_time, secondary_reload_time;
    float damage;
    MTasker<> mtasker;
};




#endif
