#ifndef DRONE_H
#define DRONE_H

#include <landscape.h>
#include <modules/math/Vector.h>
#include <modules/math/Matrix.h>
#include <modules/model/model.h>
#include "ai.h"
#include <modules/flight/autopilot.h>
#include <modules/flight/flightinfo.h>
#include <modules/engines/controls.h>
#include <modules/actors/simpleactor.h>
#include <modules/collide/CollisionManager.h>

class FlightEngine2;

class Drone : public SimpleActor, public Collide::Collidable {
public:
    Drone(Ptr<IGame> thegame);

    virtual void action();

    virtual void draw();

    virtual void applyDamage(float damage, int domain);
    virtual float getRelativeDamage();

    virtual void integrate(float delta_t, Transform * transforms);
    virtual void update(float delta_t, const Transform * new_transforms);

    void explode();

    void fireBullet();
    void fireSmartMissile();
    void fireDumbMissile();
    
private:
    JRenderer * renderer;
    Ptr<ITerrain> terrain;

    // ai stuff
    Personality personality;
    std::list<Ptr<Idea> > ideas;
    Ptr<Idea> current_idea;
    Ptr<PatrolIdea> patrol_idea;

    // 3d model
    Ptr<Model> model;

    // flight stuff
    Ptr<FlightEngine2> engine;
    FlightInfo flight_info;
    Ptr<DroneControls> drone_controls;
    AutoPilot auto_pilot;

    int cannon_num, smart_launcher_num, dumb_launcher_num;
    float primary_reload_time, secondary_reload_time;
    float damage;
};




#endif
