#ifndef DRONE_H
#define DRONE_H

#include <landscape.h>
#include <mtasker.h>
#include <modules/math/Vector.h>
#include <modules/math/Matrix.h>
#include <modules/model/model.h>
#include <modules/model/Skeleton.h>
#include <modules/flight/autopilot.h>
#include <modules/flight/flightinfo.h>
#include <modules/engines/controls.h>
#include <modules/actors/simpleactor.h>
#include <modules/collide/CollisionManager.h>
#include <modules/weaponsys/Armament.h>
#include <modules/scripting/IoIncludes.h>


class RigidEngine;
struct Idea;
class PatrolIdea;
struct Rating;
class Targeter;
class SoundSource;
struct Context;

namespace Effectors { class Wheel; }

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

class Drone : public SimpleActor, public Collide::Collidable, virtual public SigObject {
public:
    Drone(Ptr<IGame> thegame, IoObject * io_peer=0);
    virtual ~Drone();

    virtual void onLinked();
    virtual void onUnlinked();
    
    virtual void action();
    virtual void kill();

    virtual void draw();

    virtual void applyDamage(float damage, int domain, Ptr<IProjectile>);
    virtual float getRelativeDamage();

    virtual void integrate(float delta_t, Transform * transforms);
    virtual void update(float delta_t, const Transform * new_transforms);
    
    virtual int getNumViews();
    virtual Ptr<IView> getView(int n);

    void explode(bool deadly=true);

    virtual bool hasControlMode(ControlMode);
    virtual void setControlMode(ControlMode);
    
private:
    void drawWheels();
    
private:
    JRenderer * renderer;
    Ptr<ITerrain> terrain;
    Ptr<SoundSource> engine_sound_src;

    // ai stuff
    Ptr<Context> context;
    Personality personality;
    std::list<Ptr<Idea> > ideas;
    Ptr<Idea> current_idea;
    Ptr<PatrolIdea> patrol_idea;

    // 3d model
    Ptr<Skeleton> skeleton;
    Ptr<Model> inside_model;
    Ptr<Model> wheel_model;

    // flight stuff
    Ptr<RigidEngine> engine;
    FlightInfo flight_info;
    Ptr<FlightControls> flight_controls;
    AutoPilot auto_pilot;
    Ptr<Effectors::Wheel> wheels[3];

    float damage;
    MTasker<> mtasker;
};




#endif
