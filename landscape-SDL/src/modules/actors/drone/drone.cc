#include "drone.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <modules/actors/fx/smokecolumn.h>
#include <modules/actors/fx/explosion.h>
#include <modules/actors/projectiles/bullet.h>
#include <modules/actors/projectiles/dumbmissile.h>
#include <modules/actors/projectiles/smartmissile.h>
#include <modules/clock/clock.h>
#include <modules/engines/flightengine2.h>

#define PI 3.14159265358979323846

#define RADIUS 10.0f

#define BULLET_SPEED 900.0f
#define DUMBMISSILE_SPEED 200.0f
#define SMARTMISSILE_SPEED 80.0f

#define BULLET_RANGE 1000.0f
#define BULLET_TTL (BULLET_RANGE / BULLET_SPEED)

#define PRIMARY_RELOAD_TIME 0.03f
#define SECONDARY_RELOAD_TIME 0.3f;

#define RAND ((float) rand() / (float) RAND_MAX)
#define RAND2 ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)

Drone::Drone(Ptr<IGame> thegame)
: SimpleActor(thegame),
  renderer(thegame->getRenderer()),
  terrain(thegame->getTerrain()), damage(0)
{
    setTargetInfo(new TargetInfo(
        "Drone", RADIUS, TargetInfo::CLASS_AIRCRAFT));

    drone_controls = new DroneControls;
    engine = new FlightEngine2(thegame);
    setEngine(engine);
    engine->setFlightControls(drone_controls);

    // Prepare collidable
    setBoundingGeometry(
        thegame->getCollisionMan()->queryGeometry(
            thegame->getConfig()->query("Drone_model_bounds")));
    setRigidBody(engine);
    setActor((IActor*)this);

    thegame->getCollisionMan()->add(this);

    personality.randomize();

    std::string model_file = thegame->getConfig()->query("Drone_model_file");
    std::string model_path = thegame->getConfig()->query("Drone_model_path");
    model = thegame->getModelMan()->query(model_file);
    //model = new Model(*thegame->getTexMan(), objfile, model_path.c_str());

    patrol_idea =  new PatrolIdea(&auto_pilot, &flight_info,
            Vector(0,0,0), 10000, terrain);
    ideas.push_back(patrol_idea);
    ideas.push_back( new AttackIdea(thegame, this, &auto_pilot, &flight_info,
                                    drone_controls));
    ideas.push_back( new EvadeTerrainIdea(&auto_pilot, &flight_info) );

    drone_controls->setThrottle(1);
    drone_controls->setElevator(0);
    drone_controls->setAileronAndRudder(0);

    cannon_num = dumb_launcher_num = smart_launcher_num = 0;
    damage = 0;
    primary_reload_time = secondary_reload_time = 0;

    views.clear();
    Vector pilot_pos(0, 1.5f, 3);
    views.push_back(new RelativeView(
            *this,
            pilot_pos,
            Vector(1,0,0),
            Vector(0,1,0),
            Vector(0,0,1)));
    views.push_back(new RelativeView(
            *this,
            pilot_pos,
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
    views.push_back(new RelativeView(
            *this,
            Vector(0, 10, -30),
            Vector(1,0,0),
            Vector(0,1,0),
            Vector(0,0,1)));
    views.push_back(new RelativeView(
            *this,
            Vector(0, 10, 30),
            Vector(-1,0,0),
            Vector(0,1,0),
            Vector(0,0,-1)));
}

void Drone::action() {
    float delta_t = thegame->getClock()->getStepDelta();

    flight_info.update(delta_t, *this, *terrain);

    float best_value=0;
    Ptr<Idea> best_idea;
    for(std::list<Ptr<Idea> >::iterator i=ideas.begin(); i!=ideas.end(); i++) {
        Rating r = (*i)->rate();
//         ls_message("rating: attack=%f defense=%f order=%f opportunity=%f\n"
//                 "\tnecessity=%f danger=%f\n",
//                 r.attack, r.defense, r.order, r.opportunity,
//                 r.necessity, r.danger);
        float value = personality.evaluate(r);
//         ls_message("value = %f\n", value);
        if (value > best_value) {
            best_value = value;
            best_idea = *i;
        }
    }
    if (current_idea && best_idea != current_idea) current_idea->postpone();
    current_idea = best_idea;
    if (current_idea) {
        current_idea->realize();
        getTargetInfo()->setTargetInfo(current_idea->info());
    }

    //flight_info.dump();
    auto_pilot.fly(flight_info, *drone_controls);
    SimpleActor::action();

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

    Vector p = getLocation();
    if (p[1] < terrain->getHeightAt(p[0],p[2]) + 5.0) {
        p[1] = terrain->getHeightAt(p[0],p[2]) + 5.0;
        setLocation(p);
        explode();
    }
    // Todo : collision detection
}


#define MAX_MODEL_DISTANCE 3000.0f
#define MAX_POINT_DISTANCE 10000.0f

void Drone::draw() {
    std::deque<Vector> & path = patrol_idea->getPath();
    std::deque<Vector>::iterator it = path.begin();

    renderer->enableSmoothShading();
    renderer->disableTexturing();
    renderer->disableFog();
    renderer->begin(JR_DRAWMODE_CONNECTED_LINES);
    renderer->setColor(Vector(0,1,0));
    renderer->setAlpha(1);
    while (it != path.end()) {
        Vector v = *it;
        //v[1] = getLocation()[1];
        //v[1] = terrain->getHeightAt(v[0], v[2]) + 400.0f;
        ++it;
        *renderer << v;
    }
    renderer->end();
    renderer->enableFog();



    float frustum[6][4];
    float dist = 0.0;

    Vector p = engine->getLocation();

    thegame->getCamera()->getFrustumPlanes(frustum);
    for(int plane=0; plane<6; plane++) {
        float d = 0;
        for(int i=0; i<3; i++) d += frustum[plane][i]*p[i];
        d += frustum[plane][3];
        if (d < -RADIUS) return;
        if (plane == PLANE_MINUS_Z) dist = d;
    }

    if (dist > MAX_POINT_DISTANCE) return;
    if (dist > MAX_MODEL_DISTANCE) {
        renderer->disableTexturing();
        renderer->enableAlphaBlending();
        renderer->begin(JR_DRAWMODE_POINTS);
        renderer->setColor(Vector(0,0,0));
        renderer->setAlpha(1.0 - (dist-MAX_MODEL_DISTANCE) /
                (MAX_POINT_DISTANCE - MAX_MODEL_DISTANCE));
        renderer->vertex(p);
        renderer->end();
        return;
    }

    Matrix Translation = TranslateMatrix<4,float>(p);

    Vector right, up, front;
    getOrientation(&up, &right, &front);
    Matrix Rotation    = Matrix::Hom(
        MatrixFromColumns(right, up, front));

    Matrix Mmodel  = Translation * Rotation;
    renderer->setCullMode(JR_CULLMODE_CULL_NEGATIVE);
    model->draw(*renderer, Mmodel, Rotation);

}

// Our drone has been hit ...
void Drone::applyDamage(float damage, int domain) {
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

float Drone::getRelativeDamage() {
    return std::max(0.0f, std::min(1.0f, this->damage));
}

#define MAX_EXPLOSION_SIZE 6.0
#define MIN_EXPLOSION_SIZE 1.0
#define MAX_EXPLOSION_DISTANCE 40.0
#define NUM_EXPLOSIONS 15
#define MAX_EXPLOSION_AGE -2.0
void Drone::explode() {
    state=DEAD;
    Vector p = getLocation();
    for(int i=0; i<NUM_EXPLOSIONS; i++) {
        Vector v = p + RAND * MAX_EXPLOSION_DISTANCE *
                Vector(RAND2, RAND2, RAND2).normalize();
        float size = MIN_EXPLOSION_SIZE +
                RAND * (MAX_EXPLOSION_SIZE - MIN_EXPLOSION_SIZE);
        double time = RAND * MAX_EXPLOSION_AGE;
        thegame->addActor(new Explosion(thegame, v, size, time));
    }
    thegame->getCollisionMan()->remove(this);

    // HACK: create new drone somewhere
    Ptr<Drone> drone = new Drone(thegame);
    float r = RAND;
    if (3*r<1) drone->setFaction(Faction::basic_factions.faction_a);
    else if (3*r<2) drone->setFaction(Faction::basic_factions.faction_b);
    else drone->setFaction(Faction::basic_factions.faction_c);
    Vector p0(RAND, 0, RAND);
    p0*=36000;
    p0[1] = terrain->getHeightAt(p0[0], p0[2]) + 500;
    drone->setLocation(p0);
    thegame->addActor(drone);
}


void Drone::integrate(float delta_t, Transform * transforms) {
    engine->integrate(delta_t, transforms);
}

void Drone::update(float delta_t, const Transform * new_transforms) {
    engine->update(delta_t, new_transforms);
}

void Drone::fireBullet()
{
    static const Vector cannon[4]={
        Vector(-1.7, -1.0, 10.0),
        Vector( 1.7, -1.0, 10.0),
        Vector(-1.7, +1.0, 10.0),
        Vector( 1.7, +1.0, 10.0)
    };

    Ptr<IActor> target = thegame->getGunsight()->getCurrentTarget();
    Ptr<Bullet> projectile( new Bullet(&*thegame) );
    projectile->setTTL(BULLET_TTL);

    Vector start = getLocation()
        + engine->getState().q.rot(cannon[cannon_num++]);
    Vector move(getMovementVector());

    if (cannon_num==4) cannon_num=0;

    Ptr<SoundSource> snd_src = thegame->getSoundMan()->requestSource();
    snd_src->setPosition(start);
    snd_src->play(thegame->getSoundMan()->querySound(
            thegame->getConfig()->query("Player_cannon_sound")));

    move += engine->getState().q.rot(
        BULLET_SPEED * Vector(0,0,1));

    thegame->addActor(projectile);
    projectile->shoot(start, move, getFrontVector());
}

void Drone::fireDumbMissile()
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

void Drone::fireSmartMissile()
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
