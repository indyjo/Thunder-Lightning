#include <cstdlib>
#include <landscape.h>
#include <modules/engines/controls.h>
#include <modules/flight/flightinfo.h>
#include <modules/flight/autopilot.h>
#include <modules/actors/projectiles/aiminghelper.h>
#include <modules/model/model.h>
#include <modules/actors/simpleactor.h>
#include <modules/collide/CollisionManager.h>
#include <interfaces/IPlayer.h>


class FlightEngine2;

class Player: public SimpleActor,
              public IPlayer,
              public Collide::Collidable,
              virtual public SigObject
{
public:
    Player(Ptr<IGame> thegame);

    virtual void action();
    virtual void draw();

    virtual void setThrottle(float);
    virtual float getThrottle();

    virtual void hitTarget(float damage);

    virtual void integrate(float delta_t, Transform * transforms);
    virtual void update(float delta_t, const Transform * new_transforms);

private:
    void doFlight();

    void explode();

    void strafeForward(bool);
    void strafeBackward(bool);
    void strafeLeft(bool);
    void strafeRight(bool);

    Vector getStrafeVector(const Vector & front, const Vector & right);

    void toggleAutoPilot();

    void fireBullet();
    void fireSmartMissile();
    void fireDumbMissile();
    
    enum Event {
        CYCLE_PRIMARY,
        CYCLE_SECONDARY,
        FIRE_PRIMARY,
        RELEASE_PRIMARY,
        FIRE_SECONDARY,
        RELEASE_SECONDARY
    };
    void event(Event e);

private:
    JRenderer *renderer;
    Ptr<ITerrain> terrain;
    Ptr<FlightEngine2> engine;
    Ptr<DroneControls> drone_controls;
    FlightInfo flight_info;
    AutoPilot auto_pilot;
    bool auto_pilot_enabled;
    struct { bool forward, backward, right, left; } strafe;
    Ptr<AimingHelper> helper;
    // 3d model
    Ptr<Model> model;
    int cannon_num, smart_launcher_num, dumb_launcher_num;
    float primary_reload_time, secondary_reload_time;
    float damage;
};
