#include "player.h"
#include <sigc++/bind.h>
#include <modules/engines/flightengine2.h>
#include <modules/clock/clock.h>
#include <modules/actors/projectiles/bullet.h>
#include <modules/actors/projectiles/dumbmissile.h>
#include <modules/actors/projectiles/smartmissile.h>
#include <modules/actors/projectiles/smartmissile2.h>
#include <modules/actors/fx/explosion.h>

#define STRAFE_RIGHT_ACCEL 2000.0f
#define STRAFE_FRONT_ACCEL 800.0f
#define BULLET_SPEED 900.0f
#define DUMBMISSILE_SPEED 200.0f
#define SMARTMISSILE_SPEED 80.0f

#define PRIMARY_RELOAD_TIME 0.03f
#define SECONDARY_RELOAD_TIME 0.3f;

#define PI 3.14159265358979323846
#define RAND ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)
#define RAND_POS ((float) rand() / (float) RAND_MAX)

using namespace std;

namespace {
    float gauss_rand(int n) {
        float res = 0.0f;
        for(int i=0; i<n; i++) res+=RAND;
        return res / n;
    }
}

Player::Player(Ptr<IGame> thegame)
:   SimpleActor(thegame)
{
    setTargetInfo(new TargetInfo(
        "Player", 10.0f, TargetInfo::CLASS_AIRCRAFT));

    this->thegame=thegame;
    renderer=thegame->getRenderer();
    terrain=thegame->getTerrain();

    drone_controls = new DroneControls;
    engine = new FlightEngine2(thegame);
    engine->setFlightControls(drone_controls);
    setEngine(engine);

    setBoundingGeometry(
        thegame->getCollisionMan()->queryGeometry(
            thegame->getConfig()->query("Player_model_bounds")));
    setRigidBody(engine);
    setActor((IActor*)this);

    thegame->getCollisionMan()->add(this);

    strafe.forward=false;
    strafe.backward=false;
    strafe.left=false;
    strafe.right=false;

    Vector p(0, INITIAL_ALTITUDE, 0);
    Vector v(0, 0, 100);
    engine->setLocation(p);
    engine->setMovementVector(v);

    drone_controls->setThrottle(1);
    drone_controls->setElevator(0);
    drone_controls->setAileronAndRudder(0);

    auto_pilot_enabled = false;

    string model_file = thegame->getConfig()->query("Player_model_file");
    string model_path = thegame->getConfig()->query("Player_model_path");
    ls_message("Player: loading model \n\t%s from \n\t%s\n",
            model_file.c_str(), model_path.c_str());
    ls_message("Opening input stream.\n");
    ifstream objfile(model_file.c_str());
    ls_message("creating model.\n");
    ls_message("file: %s\npath: %s\n", model_file.c_str(), model_path.c_str());
    model = new Model(*thegame->getTexMan(), objfile, model_path.c_str());

    views.clear();
    Vector pilot_pos(0, 1.5f, 3);
    views.push_back(new RelativeView(
            *this,
            pilot_pos,
            Vector(1,0,0),
            Vector(0,1,0),
            Vector(0,0,1)));
    /*
    views.push_back(new RelativeView(
            *this,
            Vector(0, 10, 30),
            Vector(-1,0,0),
            Vector(0,1,0),
            Vector(0,0,-1)));
    views.push_back(new RelativeView(
            *this,
            pilot_pos,
            Vector(0,0,1),
            Vector(0,1,0),
            Vector(-1,0,0)));
    views.push_back(new RelativeView(
            *this,
            pilot_pos,
            Vector(0,0,-1),
            Vector(0,1,0),
            Vector(1,0,0)));
    */
    views.push_back(new RelativeView(
            *this,
            Vector(0, 10, 30),
            Vector(-1,0,0),
            Vector(0,1,0),
            Vector(0,0,-1)));
    views.push_back(new RelativeView(
            *this,
            Vector(-30,0,15),
            Vector(0,0,-1),
            Vector(0,1,0),
            Vector(1,0,0)));
    views.push_back(new RelativeView(
            *this,
            Vector(30,0,15),
            Vector(0,0,1),
            Vector(0,1,0),
            Vector(-1,0,0)));
    views.push_back(new RelativeView(
            *this,
            Vector(0, 10, -30),
            Vector(1,0,0),
            Vector(0,1,0),
            Vector(0,0,1)));
    views.push_back(new RelativeView(
            *this,
            Vector(0, -10, 30),
            Vector(-1,0,0),
            Vector(0,1,0),
            Vector(0,0,-1)));

    cannon_num = dumb_launcher_num = smart_launcher_num = 0;
    damage = 0;
    primary_reload_time = secondary_reload_time = 0;

    ls_message("Mapping events.\n");
    EventRemapper *r=thegame->getEventRemapper();
    r->map("+forward", SigC::bind(
            SigC::slot(*this, & Player::strafeForward), true));
    r->map("-forward", SigC::bind(
            SigC::slot(*this, & Player::strafeForward), false));
    r->map("+backward", SigC::bind(
            SigC::slot(*this, & Player::strafeBackward), true));
    r->map("-backward", SigC::bind(
            SigC::slot(*this, & Player::strafeBackward), false));
    r->map("+left", SigC::bind(
            SigC::slot(*this, & Player::strafeLeft), true));
    r->map("-left", SigC::bind(
            SigC::slot(*this, & Player::strafeLeft), false));
    r->map("+right", SigC::bind(
            SigC::slot(*this, & Player::strafeRight), true));
    r->map("-right", SigC::bind(
            SigC::slot(*this, & Player::strafeRight), false));
    r->map("cycle-primary", SigC::bind(
            SigC::slot(*this, & Player::event), CYCLE_PRIMARY));
    r->map("cycle-secondary", SigC::bind(
            SigC::slot(*this, & Player::event), CYCLE_SECONDARY));
    r->map("+primary", SigC::bind(
            SigC::slot(*this, & Player::event), FIRE_PRIMARY));
    r->map("-primary", SigC::bind(
            SigC::slot(*this, & Player::event), RELEASE_PRIMARY));
    r->map("+secondary", SigC::bind(
            SigC::slot(*this, & Player::event), FIRE_SECONDARY));
    r->map("-secondary", SigC::bind(
            SigC::slot(*this, & Player::event), RELEASE_SECONDARY));

    r->map("throttle0", SigC::bind(
            SigC::slot(*this, & Player::setThrottle), 0.0f));
    r->map("throttle1", SigC::bind(
            SigC::slot(*this, & Player::setThrottle), 0.11f));
    r->map("throttle2", SigC::bind(
            SigC::slot(*this, & Player::setThrottle), 0.22f));
    r->map("throttle3", SigC::bind(
            SigC::slot(*this, & Player::setThrottle), 0.33f));
    r->map("throttle4", SigC::bind(
            SigC::slot(*this, & Player::setThrottle), 0.44f));
    r->map("throttle5", SigC::bind(
            SigC::slot(*this, & Player::setThrottle), 0.55f));
    r->map("throttle6", SigC::bind(
            SigC::slot(*this, & Player::setThrottle), 0.66f));
    r->map("throttle7", SigC::bind(
            SigC::slot(*this, & Player::setThrottle), 0.77f));
    r->map("throttle8", SigC::bind(
            SigC::slot(*this, & Player::setThrottle), 0.88f));
    r->map("throttle9", SigC::bind(
            SigC::slot(*this, & Player::setThrottle), 1.0f));

    r->map("autopilot", SigC::slot(*this, & Player::toggleAutoPilot));

    ls_message("At end of Player init.\n");
}


void Player::action()
{
    doFlight();
    float delta_t = thegame->getClock()->getStepDelta();
    /*ls_message("Player position: x=%f y=%f z=%f. Speed=%f\n",
           position.point.x,position.point.y,position.point.z,speed);*/

    Ptr<IActor> target = thegame->getGunsight()->getCurrentTarget();
    if(target) {
        if (!helper) {
            helper = new AimingHelper(thegame);
            thegame->addActor(helper);
        }
        helper->update(delta_t,
                target->getLocation(), target->getMovementVector(),
                getLocation(), getMovementVector(),
                BULLET_SPEED);
    } else if (helper) {
        helper->kill();
        helper = 0;
    }

    if (primary_reload_time > 0) primary_reload_time -= delta_t;
    if (secondary_reload_time > 0) secondary_reload_time -= delta_t;

    if (drone_controls->getFirePrimary() && primary_reload_time <= 0) {
        fireBullet();
        primary_reload_time = PRIMARY_RELOAD_TIME;
    }
    if (drone_controls->getFireSecondary() && secondary_reload_time <= 0) {
        drone_controls->setFireSecondary(false);
        secondary_reload_time = SECONDARY_RELOAD_TIME;
        switch(drone_controls->getSecondary()) {
        case 0:
            fireDumbMissile();
            break;
        case 1:
            fireSmartMissile();
            break;
        }
    }
}

void Player::draw()
{
    //if (!(thegame->getCamPos() == this)) {
    if (true) {
        Vector p = getLocation();
        Matrix Translation = TranslateMatrix<4,float>(p);

        Vector right, up, front;
        getOrientation(&up, &right, &front);
        Matrix Rotation = Matrix::Hom(
            MatrixFromColumns<float>(right, up, front));

        Matrix Mmodel  = Translation * Rotation;
        renderer->setCullMode(JR_CULLMODE_CULL_NEGATIVE);
        model->draw(*renderer, Mmodel, Rotation);
    }
}

void Player::doFlight()
{
    float mx, my;
    int buttons;

    thegame->getMouseState(&mx, &my, &buttons);
    double delta_t = thegame->getTimeDelta() / 1000.0;
    mx/=delta_t;
    my/=delta_t;

    mx = max(-1.0, min( 1.0, mx / 100.0));
    my = max(-1.0, min( 1.0, -my / 100.0));

    // dead zone
#define DZ 0.1
//     if (abs(mx)<DZ) mx = 0;
//     else {
//         if (mx>0) mx = (mx-DZ) / (1.0-DZ);
//         else      mx = (mx+DZ) / (1.0-DZ);
//     }
//     if (abs(my)<DZ) my = 0;
//     else {
//         if (my>0) my = (my-DZ) / (1.0-DZ);
//         else      my = (my+DZ) / (1.0-DZ);
//     }

    mx *= abs(mx);
    my *= abs(my);

    //ls_message("Elevator: %f Aileron/Rudder: %f\n", my, mx);


    //drone_controls->setRudder( 0.2*mx );
    //drone_controls->setAileron( mx );
    //drone_controls->setElevator( my );
    //drone_controls.setThrottle( 1 );

    drone_controls->setRudder( thegame->getEventRemapper()->getAxis("rudder") );
    drone_controls->setAileron( thegame->getEventRemapper()->getAxis("aileron") );
    drone_controls->setElevator( -thegame->getEventRemapper()->getAxis("elevator") );
    drone_controls->setThrottle(
        0.5 - 0.5*thegame->getEventRemapper()->getAxis("throttle") );

    flight_info.update(delta_t, *this, *terrain);
    if (auto_pilot_enabled) auto_pilot.fly(flight_info, *drone_controls);

    engine->applyLinearAcceleration(getStrafeVector(
        getFrontVector(), getRightVector()));
    // Let the flight engine do the physics work
    SimpleActor::action();
}

void Player::setThrottle(float throttle) {
    ls_message("Player: setting throttle to %f\n", throttle);
    drone_controls->setThrottle(throttle);
}

float Player::getThrottle() {
    return drone_controls->getThrottle();
}

// Our drone has been hit ...
void Player::hitTarget(float damage) {
    if (this->damage < 0.7 && this->damage+damage>0.7) {
        SmokeColumn::Params params;
        params.interval=0.1;
        Ptr<FollowingSmokeColumn> smoke =
                new FollowingSmokeColumn(thegame, params);
        smoke->follow(this);
        thegame->addActor(smoke);
    }
    this->damage += damage;
    engine->setMaxThrust( 40000 * (1-damage) );
    if (this->damage > 1.0) explode();
}

#define MAX_EXPLOSION_SIZE 6.0
#define MIN_EXPLOSION_SIZE 1.0
#define MAX_EXPLOSION_DISTANCE 40.0
#define NUM_EXPLOSIONS 15
#define MAX_EXPLOSION_AGE -2.0
void Player::explode() {
    state=DEAD;
    Vector p = getLocation();
    for(int i=0; i<NUM_EXPLOSIONS; i++) {
        Vector v = p + RAND_POS * MAX_EXPLOSION_DISTANCE *
                Vector(RAND, RAND, RAND).normalize();
        float size = MIN_EXPLOSION_SIZE +
                RAND_POS * (MAX_EXPLOSION_SIZE - MIN_EXPLOSION_SIZE);
        double time = RAND_POS * MAX_EXPLOSION_AGE;
        thegame->addActor(new Explosion(thegame, v, size, time));
    }
    thegame->getCollisionMan()->remove(this);
}


void Player::integrate(float delta_t, Transform * transforms) {
    engine->integrate(delta_t, transforms);
}

void Player::update(float delta_t, const Transform * new_transforms) {
    engine->update(delta_t, new_transforms);
}



void Player::strafeForward(bool active)
{
    strafe.forward=active;
}
void Player::strafeBackward(bool active)
{
    strafe.backward=active;
}
void Player::strafeRight(bool active)
{
    strafe.right=active;
}
void Player::strafeLeft(bool active)
{
    strafe.left=active;
}

Vector Player::getStrafeVector(const Vector & front, const Vector & right)
{
    Vector v(0,0,0);
    if (strafe.forward) v += front * STRAFE_FRONT_ACCEL;
    if (strafe.backward) v -= front * STRAFE_FRONT_ACCEL;
    if (strafe.right) v += right * STRAFE_RIGHT_ACCEL;
    if (strafe.left) v -= right * STRAFE_RIGHT_ACCEL;
    return v;
}

void Player::toggleAutoPilot() {
    auto_pilot_enabled = !auto_pilot_enabled;
    if (auto_pilot_enabled) {
        auto_pilot.setMode(AP_HEIGHT_MASK | AP_COURSE_MASK | AP_SPEED_MASK);
        auto_pilot.setTargetHeight(flight_info.getCurrentHeight());
        auto_pilot.setTargetCourse(flight_info.getCurrentCourse());
        auto_pilot.setTargetSpeed(flight_info.getCurrentSpeed());
        ls_message("Player: Enabling autopilot.\n");
        ls_message("height: %4.0fm course: %3.0f° speed: %4.0fkm/h\n",
                flight_info.getCurrentHeight(),
                flight_info.getCurrentCourse() * 360.0 / PI,
                flight_info.getCurrentSpeed() * 3.6);
    } else {
        auto_pilot.reset();
        drone_controls->setRudder(0);
    }
}


void Player::fireBullet()
{
    static const Vector cannon[4]={
        Vector(-1.7, -1.0, 10.0),
        Vector( 1.7, -1.0, 10.0),
        Vector(-1.7, +1.0, 10.0),
        Vector( 1.7, +1.0, 10.0)
    };

    Ptr<IActor> target = thegame->getGunsight()->getCurrentTarget();
    Ptr<Bullet> projectile( new Bullet(&*thegame) );

    Vector start = getLocation()
        + engine->getState().q.rot(cannon[cannon_num++]);
    Vector move(getMovementVector());
    engine->applyImpulseAt(-0.05f * BULLET_SPEED * getFrontVector(),start);

    if (cannon_num==4) cannon_num=0;

    Ptr<SoundSource> snd_src = thegame->getSoundMan()->requestSource();
    snd_src->setPosition(start);
    snd_src->play(thegame->getSoundMan()->querySound(
            thegame->getConfig()->query("Player_cannon_sound")));

    float error = gauss_rand(10);
    float x = error * RAND;
    float y = sqrt(error*error - x*x);
    move += engine->getState().q.rot(
        BULLET_SPEED * Vector(0.05f*x,0.05f*y,1));

    thegame->addActor(projectile);
    projectile->shoot(start, move, getFrontVector());
}

void Player::fireDumbMissile()
{
    static const Vector launchers[8]={
        Vector(-3.0, -1.0, 4.5),
        Vector( 3.0, -1.0, 4.5),
        Vector(-3.0,  0.5, 4.5),
        Vector( 3.0,  0.5, 4.5),
        Vector(-4.5, -1.0, 4.5),
        Vector( 4.5, -1.0, 4.5),
        Vector(-4.5,  0.5, 4.5),
        Vector( 4.5,  0.5, 4.5)
    };

    Ptr<DumbMissile> projectile( new DumbMissile(&*thegame) );

    Vector start = getLocation()
        + engine->getState().q.rot(launchers[dumb_launcher_num++]);
    Vector move(getMovementVector());

    if (dumb_launcher_num==8) dumb_launcher_num=0;

    move += engine->getState().q.rot(
        DUMBMISSILE_SPEED * Vector(0,0,1));

    thegame->addActor(projectile);
    projectile->shoot(start, move, getFrontVector());
}

void Player::fireSmartMissile()
{
    static const Vector launchers[4]={
        Vector(-3.0, 0, 8.0),
        Vector( 3.0, 0, 8.0),
        Vector(-3.5, 0, 8.0),
        Vector( 3.5, 0, 8.0)
    };

    Ptr<IActor> target = thegame->getGunsight()->getCurrentTarget();

    Ptr<SmartMissile> projectile( new SmartMissile(&*thegame, target) );
    Vector start = getLocation()
        + engine->getState().q.rot(launchers[smart_launcher_num++]);
    Vector move(getMovementVector());

    if (smart_launcher_num==4) smart_launcher_num=0;

    move += engine->getState().q.rot(
        DUMBMISSILE_SPEED * Vector(0,0,1));

    thegame->addActor(projectile);
    projectile->shoot(start, move, getFrontVector());
}


/// Handles a primitive event.
/// @param event the Event to handle
/// @see Event
void Player::event(Event event) {
    switch(event) {
    case CYCLE_PRIMARY:
        drone_controls->setPrimary(0);
        break;
    case CYCLE_SECONDARY:
        drone_controls->setSecondary(
            (drone_controls->getSecondary()+1) % 2);
        break;
    case FIRE_PRIMARY:
        drone_controls->setFirePrimary(true);
        break;
    case FIRE_SECONDARY:
        drone_controls->setFireSecondary(true);
        break;
    case RELEASE_PRIMARY:
        drone_controls->setFirePrimary(false);
        break;
    case RELEASE_SECONDARY:
        drone_controls->setFireSecondary(false);
        break;
    default:
        ls_error("Player::event: Unknown event %d\n", event);
    }
}
