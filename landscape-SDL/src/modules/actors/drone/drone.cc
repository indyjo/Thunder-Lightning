#include <cstdlib>
#include <iostream>
#include <string>
#include <sigc++/bind.h>
#include <modules/actors/fx/smokecolumn.h>
#include <modules/actors/fx/explosion.h>
#include <modules/actors/projectiles/bullet.h>
#include <modules/actors/projectiles/dumbmissile.h>
#include <modules/actors/projectiles/smartmissile.h>
#include <modules/clock/clock.h>
#include <modules/engines/flightengine2.h>
#include <modules/gunsight/gunsight.h>
#include <modules/math/SpecialMatrices.h>
#include <modules/weaponsys/Targeter.h>

#include <remap.h>
#include <sound.h>
#include <interfaces/IConfig.h>
#include <interfaces/ICamera.h>
#include <interfaces/IModelMan.h>
#include <interfaces/ITerrain.h>

#include "drone.h"
#include "ai.h"

#define PI 3.14159265358979323846

#define RADIUS 10.0f

#define CURRENT_IDEA_BONUS 0.05f

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
  terrain(thegame->getTerrain()), damage(0),
  mtasker(64*1024),
  control_mode(UNCONTROLLED)
{
    setTargetInfo(new TargetInfo(
        "Drone", RADIUS, TargetInfo::AIRCRAFT));

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
    
    targeter = new Targeter(*thegame,*this);

    personality.randomize();
    context = new Context(
    	&flight_info,
    	&auto_pilot,
    	drone_controls,
    	thegame,
    	terrain,
    	this,
    	targeter,
    	mtasker);

    std::string model_file = thegame->getConfig()->query("Drone_model_file");
    std::string model_path = thegame->getConfig()->query("Drone_model_path");
    model = thegame->getModelMan()->query(model_file);
    //model = new Model(*thegame->getTexMan(), objfile, model_path.c_str());

    patrol_idea =  new PatrolIdea(*context,Vector(0,0,0), 10000);
    ideas.push_back(patrol_idea);
    //ideas.push_back( new AttackIdea(*context));
    ideas.push_back( new EvadeTerrainIdea(*context) );
    //ideas.push_back( new CRIdea(*context));
    ideas.push_back( new Dogfight(*context));

    setControlMode(UNCONTROLLED);

    cannon_num = dumb_launcher_num = smart_launcher_num = 0;
    damage = 0;
    primary_reload_time = secondary_reload_time = 0;

    Ptr<FlexibleGunsight> gunsight = new FlexibleGunsight(thegame);
    gunsight->addDebugInfo(thegame, this);
    gunsight->addFlightModules(thegame, flight_info);
    gunsight->addBasicCrosshairs();
    gunsight->addTargeting(this, targeter);
    
    views.clear();
    Vector pilot_pos(0, 1.5f, 3);
    views.push_back(new RelativeView(
            *this,
            pilot_pos,
            Vector(1,0,0),
            Vector(0,1,0),
            Vector(0,0,1),
            gunsight));
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
            
    event_sheet = new EventSheet;
    /*
    event_sheet->map("+strafe_forward", SigC::bind(
            SigC::slot(*this, & Player::strafeForward), true));
    event_sheet->map("-strafe_forward", SigC::bind(
            SigC::slot(*this, & Player::strafeForward), false));
    event_sheet->map("+strafe_backward", SigC::bind(
            SigC::slot(*this, & Player::strafeBackward), true));
    event_sheet->map("-strafe_backward", SigC::bind(
            SigC::slot(*this, & Player::strafeBackward), false));
    event_sheet->map("+strafe_left", SigC::bind(
            SigC::slot(*this, & Player::strafeLeft), true));
    event_sheet->map("-strafe_left", SigC::bind(
            SigC::slot(*this, & Player::strafeLeft), false));
    event_sheet->map("+strafe_right", SigC::bind(
            SigC::slot(*this, & Player::strafeRight), true));
    event_sheet->map("-strafe_right", SigC::bind(
            SigC::slot(*this, & Player::strafeRight), false));
    */
    event_sheet->map("cycle-primary", SigC::bind(
            SigC::slot(*this, & Drone::event), CYCLE_PRIMARY));
    event_sheet->map("cycle-secondary", SigC::bind(
            SigC::slot(*this, & Drone::event), CYCLE_SECONDARY));
    event_sheet->map("+primary", SigC::bind(
            SigC::slot(*this, & Drone::event), FIRE_PRIMARY));
    event_sheet->map("-primary", SigC::bind(
            SigC::slot(*this, & Drone::event), RELEASE_PRIMARY));
    event_sheet->map("+secondary", SigC::bind(
            SigC::slot(*this, & Drone::event), FIRE_SECONDARY));
    event_sheet->map("-secondary", SigC::bind(
            SigC::slot(*this, & Drone::event), RELEASE_SECONDARY));
    event_sheet->map("next-target",
    		SigC::slot(*targeter, &Targeter::selectNextTarget));
    event_sheet->map("previous-target",
    		SigC::slot(*targeter, &Targeter::selectPreviousTarget));
    event_sheet->map("next-hostile-target",
    		SigC::slot(*targeter, &Targeter::selectNextHostileTarget));
    event_sheet->map("previous-hostile-target",
    		SigC::slot(*targeter, &Targeter::selectPreviousHostileTarget));
    event_sheet->map("next-friendly-target",
    		SigC::slot(*targeter, &Targeter::selectNextFriendlyTarget));
    event_sheet->map("previous-friendly-target",
    		SigC::slot(*targeter, &Targeter::selectPreviousFriendlyTarget));
    event_sheet->map("nearest-target",
    		SigC::slot(*targeter, &Targeter::selectNearestTarget));
    event_sheet->map("nearest-hostile-target",
    		SigC::slot(*targeter, &Targeter::selectNearestHostileTarget));
    event_sheet->map("nearest-friendly-target",
    		SigC::slot(*targeter, &Targeter::selectNearestFriendlyTarget));
    event_sheet->map("gunsight-target",
    		SigC::slot(*targeter, &Targeter::selectTargetInGunsight));
    		
    //event_sheet->map("autopilot", SigC::slot(*this, & Player::toggleAutoPilot));
}         

void Drone::action() {
    float delta_t = thegame->getClock()->getStepDelta();
    std::string info;
    char buf[1024];
    

    flight_info.update(delta_t, *this, *terrain);

    if (control_mode == AUTOMATIC) {
        float best_value=-1, current_value=0;
        Ptr<Idea> best_idea;
        for(std::list<Ptr<Idea> >::iterator i=ideas.begin(); i!=ideas.end(); i++) {
            Rating r = (*i)->rate();
    //         ls_message("rating: attack=%f defense=%f order=%f opportunity=%f\n"
    //                 "\tnecessity=%f danger=%f\n",
    //                 r.attack, r.defense, r.order, r.opportunity,
    //                 r.necessity, r.danger);
            float value = personality.evaluate(r);
            snprintf(buf,1024, "%s: %.2f\n", (*i)->name.c_str(), value);
            info += buf;
    //         ls_message("value = %f\n", value);
            if (*i == current_idea) {
                current_value = value;
            }
            if (value > best_value) {
                best_value = value;
                best_idea = *i;
            }
        }
        if (current_idea && best_value > current_value + CURRENT_IDEA_BONUS) {
            current_idea->postpone();
            current_idea = best_idea;
        } else if (!current_idea) {
            current_idea = best_idea;
        }
        
        current_idea->realize();
        mtasker.scheduleAll();
        
        if (current_idea) {
            getTargetInfo()->setTargetInfo(info+current_idea->info());
        }
    
        //flight_info.dump();
        auto_pilot.fly(delta_t, flight_info, *drone_controls);
    } else if (control_mode == MANUAL) {
        drone_controls->setRudder( thegame->getEventRemapper()->getAxis("rudder") );
        drone_controls->setAileron( thegame->getEventRemapper()->getAxis("aileron") );
        drone_controls->setElevator( -thegame->getEventRemapper()->getAxis("elevator") );
        drone_controls->setThrottle( thegame->getEventRemapper()->getAxis("throttle") );
        /*
        char *axes[] = 
        	{"kbd_throttle",
        		"js_throttle",
        		"js_throttle2",
        		"throttle",
        		0};
        char **axis = axes;
        while (*axis) {
        	ls_message("%s: %f\n", *axis, thegame->getEventRemapper()->getAxis(*axis));
        	axis++;
        }
        */
    }
    Vector p = getLocation();
    /*
    if (p[1] < terrain->getHeightAt(p[0],p[2]) + 5.0) {
        p[1] = terrain->getHeightAt(p[0],p[2]) + 5.0;
        setLocation(p);
        explode();
    }
    */
   	if (p[1] < terrain->getHeightAt(p[0],p[2])+20) {
	    Vector wheels[3];
	    int nwheels = sizeof(wheels)/sizeof(Vector);
	    wheels[0]=Vector(    0, -3,5);
	    wheels[1]=Vector(-1.65, -3,-0.965); 
	    wheels[2]=Vector( 1.65, -3,-0.965);
	    float forces[3] = { 40000, 30000, 30000 };
	    float dampings[3] = { 7000, 4000, 4000 };
	    float drag_long[3] = { 100, 1000, 1000 };
	    float drag_lat[3] = {1000, 0, 0 };
	    Vector up,right,front;
	    getOrientation(&up,&right,&front);
	    Matrix M(	right[0], up[0], front[0], 0,
	    			right[1], up[1], front[1], 0,
	    			right[2], up[2], front[2], 0,
	    			       0,     0,        0, 1);
	    for(int i=0; i<nwheels; ++i) {
	    	Vector w = M * wheels[i] + p;
	    	Vector v = engine->getVelocityAt(w);
	    	float h = terrain->getHeightAt(w[0],w[2]);
	    	if (w[1] < h+1.5 && w[1] > h-1.5) {
	    		float pressure = (w[1]-h+1.5)/3;
	    		engine->applyForceAt(forces[i] * pressure * up, w);
	    		engine->applyForceAt(-dampings[i]  * up * (up*v), w);
	    		//engine->applyForceAt(-pressure * drag_long[i] * front*(front*v), w);
	    		engine->applyForceAt(-pressure * drag_lat[i] * right*(right*v), w);
	    	}
	    }
   	}
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

    // Todo : collision detection
}


#define MAX_MODEL_DISTANCE 3000.0f
#define MAX_POINT_DISTANCE 10000.0f

void Drone::draw() {
    renderer->enableSmoothShading();

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
    renderer->setAlpha(1);
    renderer->setColor(Vector(1,1,1));
    model->draw(*renderer, Mmodel, Rotation);
    
    if (current_idea == patrol_idea) {
    	typedef std::deque<Vector> Path;
    	typedef Path::iterator Iter;
    	Path & path = patrol_idea->getPath();
    	
    	renderer->setColor(Vector(0,0,1));
    	renderer->begin(JR_DRAWMODE_CONNECTED_LINES);
    	for(Iter i=path.begin(); i!=path.end(); ++i) {
    		*renderer << *i;
    	}
    	renderer->end();
    }
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
    drone->setControlMode(AUTOMATIC);
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

    Ptr<SmartMissile> projectile(
    		new SmartMissile(&*thegame, targeter->getCurrentTarget()) );
    ls_message("Launching smart missile to target: %p\n",
    	&*targeter->getCurrentTarget());
    Vector start = getLocation()
        + engine->getState().q.rot(launchers[smart_launcher_num++]);
    Vector move(getMovementVector());

    if (smart_launcher_num==4) smart_launcher_num=0;

    move += engine->getState().q.rot(
        DUMBMISSILE_SPEED * Vector(0,0,1));

    thegame->addActor(projectile);
    projectile->shoot(start, move, getFrontVector());
}


bool Drone::hasControlMode(ControlMode) {
  return true;
}
void Drone::setControlMode(ControlMode m) {
    if (control_mode==MANUAL && m!=control_mode)  {
        thegame->getEventRemapper()->removeEventSheet(event_sheet);
    }
    control_mode = m;
    if (m==UNCONTROLLED) {
        drone_controls->setThrottle(1);
        drone_controls->setElevator(0);
        drone_controls->setAileronAndRudder(0);
        drone_controls->setThrottle(0);
        drone_controls->setFirePrimary(false);
        drone_controls->setFireSecondary(false);
    } else if (m==MANUAL) {
        thegame->getEventRemapper()->addEventSheet(event_sheet);
    }
}


/// Handles a primitive event.
/// @param event the Event to handle
/// @see Event
void Drone::event(Event event) {
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
        ls_error("Drone::event: Unknown event %d\n", event);
    }
}
