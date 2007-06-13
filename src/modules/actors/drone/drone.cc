#include <cstdlib>
#include <iostream>
#include <string>
#include <sigc++/bind.h>
#include <modules/actors/fx/SpecialEffects.h>
#include <modules/actors/projectiles/bullet.h>
#include <modules/actors/projectiles/dumbmissile.h>
#include <modules/actors/projectiles/smartmissile.h>
#include <modules/actors/projectiles/smartmissile2.h>
#include <modules/clock/clock.h>
#include <modules/engines/effectors.h>
#include <modules/engines/rigidengine.h>
#include <modules/gunsight/gunsight.h>
#include <modules/math/SpecialMatrices.h>
#include <modules/model/Skeleton.h>
#include <modules/model/SkeletonProvider.h>
#include <modules/weaponsys/Targeter.h>
#include <modules/weaponsys/Cannon.h>
#include <modules/weaponsys/ProjectileLauncher.h>
#include <modules/scripting/IoScriptingManager.h>
#include <modules/scripting/mappings.h>
#include <modules/engines/ChasingEngine.h>

#include <remap.h>
#include <sound.h>
#include <interfaces/IConfig.h>
#include <interfaces/ICamera.h>
#include <interfaces/IModelMan.h>
#include <interfaces/ITerrain.h>

#include "drone.h"
#include "ai.h"
#include <modules/actors/SimpleView.h>
#include <modules/actors/RelativeView.h>

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


struct TargetView: public SimpleView {
	Ptr<Targeter> targeter;
	Vector view_pos;
	
	TargetView(
		Ptr<IActor> subject,
		Vector view_pos=Vector(0,0,0),
		Ptr<Targeter> targeter=0,
		Ptr<IDrawable> gunsight=0)
	:	SimpleView(subject,gunsight),
		targeter(targeter),
		view_pos(view_pos)
	{ }
	
	virtual void getPositionAndOrientation(Vector*pos, Matrix3 *orient)
	{
	    Vector right,up,front;
	    subject->getOrientation(&up,&right,&front);
   		*orient = MatrixFromColumns(right,up,front);
	    *pos = subject->getLocation() + *orient * view_pos;
	    if (!targeter || !targeter->getCurrentTarget()) {
	    	return;
	    }
	    	
	    Vector target_pos = targeter->getCurrentTarget()->getLocation();
	    front = (target_pos - *pos).normalize();
	    right = (up % front).normalize();
	    up = (front % right).normalize();
	    *orient = MatrixFromColumns(right,up,front);
	}
};


Drone::Drone(Ptr<IGame> thegame, IoObject* io_peer_init)
: SimpleActor(thegame),
  renderer(thegame->getRenderer()),
  terrain(thegame->getTerrain()), damage(0),
  mtasker(64*1024),
  gear_lowered(false),
  engine_power(1.0)
{
	ls_message("<Drone::Drone>\n");
    if (io_peer_init)
        setIoObject(io_peer_init);
    else
        createIoObject();

	Ptr<IConfig> cfg = thegame->getConfig();
    setTargetInfo(new TargetInfo(
        "Drone", cfg->queryFloat("Drone_target_radius",15),
        TargetInfo::AIRCRAFT));

    flight_controls = new FlightControls(controls);
    engine = new RigidEngine(thegame);
    engine->construct(2000, 200000, 160000, 100000);
    engine->addEffector( new Effectors::Flight(flight_controls) );
    engine->addEffector( Effectors::Gravity::getInstance() );
    
	static const Effectors::Wheel::Params wheel_params[3] = {
	    {Vector(    0, -1.7,  2.92), Vector(1,0,0), 1, 500000, 15000, 50, 10000},
	    {Vector(-2, -1.4, -2.63), Vector(1,0,0), 1, 500000, 15000, 50, 1000},
	    {Vector( 2, -1.4, -2.63), Vector(1,0,0), 1, 500000, 15000, 50, 1000}
	};
	for (int i=0; i<3; ++i) {
	    wheels[i] = new Effectors::Wheel(terrain, thegame->getCollisionMan(), this, wheel_params[i]);
	}
	
    setEngine(engine);
    
    flight_controls->setThrottle(1.0f);

    // Load drone model and skeleton, even if there are no moving parts
    std::string skeletonfile = thegame->getConfig()->query("Drone_skeleton");
    skeleton = new Skeleton(thegame, skeletonfile);

    // Weapons stuff
    setArmament(new Armament(this, this));
    targeter = new Targeter(*thegame,*this);

    Ptr<Cannon> machinegun = new Cannon(thegame, "Vulcan", 500, 2.0/60, false);
    machinegun->addBarrel(new SkeletonProvider(skeleton, "MG_0", "origin", "z"));
    machinegun->addBarrel(new SkeletonProvider(skeleton, "MG_1", "origin", "z"));
    armament->addWeapon(0,machinegun);

    Ptr<ProjectileLauncher> smart_missile_launcher = new ProjectileLauncher(
        thegame, targeter, ProjectileFactories::factories.smart_missile_2_factory,
        "Sidewinder", 6, 2, true, true);
    smart_missile_launcher->addBarrel(
        new SkeletonProvider(skeleton, "Launcher_0", "origin", "z"));
    smart_missile_launcher->addBarrel(
        new SkeletonProvider(skeleton, "Launcher_1", "origin", "z"));
    armament->addWeapon(1,smart_missile_launcher);

    Ptr<ProjectileLauncher> dumb_missile_launcher = new ProjectileLauncher(
        thegame, targeter, ProjectileFactories::factories.dumb_missile_factory,
        "Hydra", 40, 0.5);
    dumb_missile_launcher->addBarrel(
        new SkeletonProvider(skeleton, "Launcher_0", "origin", "z"));
    dumb_missile_launcher->addBarrel(
        new SkeletonProvider(skeleton, "Launcher_1", "origin", "z"));
    armament->addWeapon(1,dumb_missile_launcher);

    // Prepare collidable
    setBoundingGeometry(
        thegame->getCollisionMan()->queryGeometry(
            cfg->query("Drone_model_bounds")));
    setRigidBody(&*engine);
    setActor(this);
    
    // Tail hook
    engine->addEffector(new Effectors::TailHook(
        thegame->getCollisionMan(),
        this,
        skeleton->getUntransformedPoint("TailHook_p0"),
        skeleton->getUntransformedPoint("TailHook_p1"),
        thegame->getConfig()->queryFloat("Drone_max_tailhook_force", 80000)
    ));

    personality.randomize();
    context = new Context(
    	this,
    	&flight_info,
    	&auto_pilot,
    	flight_controls,
    	thegame,
    	terrain,
    	targeter,
        *armament,
    	mtasker);

    std::string inside_model_file = cfg->query("Drone_inside_model_file");
    inside_model = thegame->getModelMan()->query(inside_model_file);

    patrol_idea =  new PatrolIdea(*context,Vector(0,0,0), 10000);
    ideas.push_back(patrol_idea);
    //ideas.push_back( new AttackIdea(*context));
    ideas.push_back( new EvadeTerrainIdea(*context) );
    //ideas.push_back( new CRIdea(*context));
    ideas.push_back( new Dogfight(*context));

    setControlMode(UNCONTROLLED);

    damage = 0;

    mapTargeterEvents();
    mapArmamentEvents();
    
    Ptr<EventSheet> sheet = getEventSheet();
    sheet->map("landing-gear", SigC::slot(*this, &Drone::toggleLandingGear));
    		
	ls_message("</Drone::Drone>\n");
}

Drone::~Drone() {
	ls_message("<Drone::~Drone()>\n");
	ls_message("</Drone::~Drone()>\n");
}

void Drone::onLinked() {
    SimpleActor::onLinked();
    thegame->getCollisionMan()->add(this);
    
    engine_sound_src = thegame->getSoundMan()->requestSource();
    engine_sound_src->setPosition(getLocation());
    engine_sound_src->setVelocity(getMovementVector());
    engine_sound_src->setLooping(true);
    engine_sound_src->setGain(thegame->getConfig()->queryFloat("Drone_engine_gain"));
    engine_sound_src->play(thegame->getSoundMan()->querySound(
            thegame->getConfig()->query("Drone_engine_sound")));
            
}

void Drone::onUnlinked() {
    SimpleActor::onUnlinked();
    thegame->getCollisionMan()->remove(this);
    
    engine_sound_src->stop();
}

void Drone::action() {
	if (!isAlive()) return;
	
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
        auto_pilot.fly(delta_t, flight_info, *flight_controls);
    } else if (control_mode == MANUAL) {
        flight_controls->setRudder(    thegame->getEventRemapper()->getAxis("rudder") );
        flight_controls->setAileron(   thegame->getEventRemapper()->getAxis("aileron") );
        flight_controls->setElevator( -thegame->getEventRemapper()->getAxis("elevator") );
        flight_controls->setThrottle(  thegame->getEventRemapper()->getAxis("throttle") );
        flight_controls->setBrake( thegame->getEventRemapper()->getAxis("brake") );
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
    
    setLandingGear(flight_controls->isGearLowered());

    // set brake factors on main landing gear
    wheels[0]->getParams().drag_long = 50 + 600*controls->getFloat("brake", 0);
    wheels[1]->getParams().drag_long = 50 + 950*controls->getFloat("brake", 0);
    wheels[2]->getParams().drag_long = 50 + 950*controls->getFloat("brake", 0);
    
    // rotate the front wheel according to rudder
    static const float max_wheel_turn = 60*3.141593f/180; // 60 degrees max
    wheels[0]->getParams().axis = Vector(
        cos(max_wheel_turn * controls->getFloat("rudder")),
        sin(max_wheel_turn * controls->getFloat("rudder")),
        0);
        
    // cheap-ass crash detection
    Vector p = getLocation();
    if (p[1] < terrain->getHeightAt(p[0],p[2])) {
        p[1] = terrain->getHeightAt(p[0],p[2]);
        setLocation(p);
        explode();
    }
    
    // cheap-ass damage modelling
    if (damage > 0.7) {
    	flight_controls->setRudder(1);
    	flight_controls->setAileron(-1);
    	flight_controls->setElevator(-0.2);
    }
    
    SimpleActor::action();
    
    // sound
    float r = pow(2.0f, -delta_t/1.0f);
    engine_power = r*engine_power + (1-r)*flight_controls->getThrottle();
    float pitch_lowest = thegame->getConfig()->queryFloat("Drone_engine_pitch_lowest", 0.5);
    float pitch_highest = thegame->getConfig()->queryFloat("Drone_engine_pitch_highest", 2.0);
    float gain_lowest = thegame->getConfig()->queryFloat("Drone_engine_gain_lowest", 0.2);
    float gain_highest = thegame->getConfig()->queryFloat("Drone_engine_gain_highest", 1.0);
    engine_sound_src->setPitch(pitch_lowest + engine_power*(pitch_highest-pitch_lowest));
    engine_sound_src->setGain(gain_lowest + engine_power*(gain_highest-gain_lowest));

    engine_sound_src->setPosition(getLocation());
    engine_sound_src->setVelocity(getMovementVector());
}

void Drone::kill() {
	targeter->clearCurrentTarget();
	ideas.clear();
	current_idea=0;
	patrol_idea=0;
    SimpleActor::kill();
}

#define MAX_MODEL_DISTANCE 3000.0f
#define MAX_POINT_DISTANCE 10000.0f

void Drone::draw() {
	if (!isAlive()) return;
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
    
    dist /= thegame->getCamera()->getFocus();

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
        renderer->disableAlphaBlending();
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
    
    renderer->enableLighting();

    Vector pilot_pos = skeleton->getPoint("PilotSeat");
	Vector cam_pos = thegame->getCamera()->getLocation();
    if ((pilot_pos-cam_pos).lengthSquare()<1) {
        //TODO: Add cockpit interior drawing
        inside_model->draw(*renderer, Mmodel, Rotation);
    } else {
        skeleton->draw(*renderer);
    }
    
    if (isLandingGearLowered()) {
        drawWheels();
    }
    
    renderer->disableLighting();
    
    if (current_idea == patrol_idea && thegame->debugMode()) {
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
void Drone::applyDamage(float damage, int domain, Ptr<IProjectile> projectile) {
	if (!isAlive()) return;
    SimpleActor::applyDamage(damage,domain,projectile);

	// TODO: This all belongs into an Io script
    if (projectile->getSource()) {
		Ptr<IActor> src = projectile->getSource();
		float dist2 = (src->getLocation()-getLocation()).lengthSquare();
		if (src->getFaction()->getAttitudeTowards(getFaction()) != Faction::FRIENDLY)
			targeter->setCurrentTarget(src);
	}
    if (this->damage < 0.5 && this->damage+damage>0.5) {
    	explode(false);
    }
    this->damage += damage;
    //engine->setMaxThrust( 40000 * (1-damage) );
    if (this->damage > 1.0) explode();
}

float Drone::getRelativeDamage() {
    return std::max(0.0f, std::min(1.0f, this->damage));
}

int Drone::getNumViews() {
	return 6;
}

Ptr<IView> Drone::getView(int n) {
	Vector pilot_pos = thegame->getConfig()->queryVector(
		"Drone_pilot_pos", Vector(0,0,0));
    Ptr<SimpleActor> chaser = new SimpleActor(thegame);
    
    Transform xform(Quaternion(1,0,0,0), pilot_pos);
    chaser->setEngine(new ChasingEngine(thegame,this, 0.0f, 0.1f, xform));
    thegame->addWeakActor(chaser);

    Ptr<FlexibleGunsight> gunsight1 = new FlexibleGunsight(thegame);
	gunsight1->addDebugInfo(thegame, this);
	gunsight1->addFlightModules(thegame, flight_info);
	gunsight1->addBasicCrosshairs();
	gunsight1->addTargeting(this, targeter);
	gunsight1->addDirectionOfFlight(this);
    gunsight1->addArmamentToScreen(thegame, armament, 0);
    gunsight1->addArmamentToScreenRight(thegame, armament, 1);
    gunsight1->addMissileWarning(thegame, this);
    gunsight1->addInfoMessage(thegame);

    Ptr<FlexibleGunsight> gunsight2 = new FlexibleGunsight(thegame);
	gunsight2->addDebugInfo(thegame, this);
	gunsight2->addTargeting(this, targeter);
	gunsight2->addDirectionOfFlight(this);
    gunsight2->addArmamentToScreen(thegame, armament, 0);
    gunsight2->addArmamentToScreenRight(thegame, armament, 1);
    gunsight2->addMissileWarning(thegame, this);
    gunsight2->addInfoMessage(thegame);

	switch(n) {
    case 0:
    	{
            return new RelativeView(chaser,chaser,this,gunsight1);
    	}
    case 1:
        {
            Ptr<RelativeView> view = new RelativeView(chaser, chaser, this, gunsight2);
            view->setViewOffset(
                Vector(0, 2, 12),
                Vector(-1,0,0),
                Vector(0,1,0),
                Vector(0,0,-1));
            chaser->setEngine(new ChasingEngine(thegame,this, 0.0f, 0.5f, xform));
            return view;
        }
    case 2:
        {
            Ptr<RelativeView> view = new RelativeView(chaser, chaser, this, gunsight2);
            view->setViewOffset(
                Vector(0,0,0),
                Vector(0,0,1),
                Vector(0,1,0),
                Vector(-1,0,0));
            return view;
        }
    case 3:
        {
            Ptr<RelativeView> view = new RelativeView(chaser, chaser, this, gunsight2);
            view->setViewOffset(
                Vector(0,0,0),
                Vector(0,0,-1),
                Vector(0,1,0),
                Vector(1,0,0));
            return view;
        }
    case 4:
        {
            Ptr<RelativeView> view = new RelativeView(chaser, chaser, this, gunsight2);
            view->setViewOffset(
                Vector(0, 3, -8),
                Vector(1,0,0),
                Vector(0,1,0),
                Vector(0,0,1));
            chaser->setEngine(new ChasingEngine(thegame,this, 0.15f, 0.5f, xform));
            return view;
        }
    case 5:
    	{
            Ptr<TargetView> target_view = new TargetView(this, pilot_pos, targeter);
            Transform xform(Quaternion(1,0,0,0), Vector(0,0,0));
            chaser->setEngine(new ChasingEngine(thegame, target_view, 0.0f, 0.1f, xform));
            Ptr<RelativeView> view = new RelativeView(chaser, chaser, this, gunsight2);
	    	return view;
    	}
    default:
    	return 0;
	}
}

void Drone::explode(bool deadly) {
	if (deadly) {
	    kill();
	    aircraftFinalExplosion(thegame, this);
	} else {
        aircraftFirstExplosion(thegame, this);
    }
}


void Drone::integrate(float delta_t, Transform * transforms) {
    engine->integrate(delta_t, transforms);
}

void Drone::update(float delta_t, const Transform * new_transforms) {
    engine->update(delta_t, new_transforms);
    skeleton->setBoneTransform("Body",new_transforms[0]);
}


bool Drone::hasControlMode(ControlMode) {
  return true;
}

void Drone::setControlMode(ControlMode m) {
    SimpleActor::setControlMode(m);
    if (m==UNCONTROLLED) {
        flight_controls->setThrottle(1);
        flight_controls->setElevator(0);
        flight_controls->setAileronAndRudder(0);
        flight_controls->setThrottle(0);
    }
}

void Drone::setLandingGear(bool lowered) {
    if (lowered == gear_lowered) {
        return;
    }
    
    gear_lowered=lowered;
    if (lowered) {
        for(int i=0; i<3; ++i) {
            engine->addEffector( wheels[i] );
        }
    } else {
        for(int i=0; i<3; ++i) {
            engine->removeEffector( wheels[i] );
        }
    }
}


void Drone::drawWheels() {
	if (!wheel_model) {
		Ptr<IConfig> cfg = thegame->getConfig();
	    std::string model_file = cfg->query("Drone_wheel_model_file");
	    wheel_model = thegame->getModelMan()->query(model_file);
	}
	
    Vector right, up, front;
    getOrientation(&up, &right, &front);
    Matrix Rotation    = Matrix::Hom(
        MatrixFromColumns(right, up, front));

    renderer->setCullMode(JR_CULLMODE_CULL_NEGATIVE);
    renderer->setAlpha(1);
    renderer->setColor(Vector(1,1,1));
    
	for(int i=0; i<3; ++i) {
		//if (!wheel_states[i].contact) continue;
    	Matrix Translation =
    		TranslateMatrix<4,float>(wheels[i]->getCurrentPos());
    	Matrix Mmodel  = Translation * Rotation;
    	wheel_model->draw(*renderer, Mmodel, Rotation);
	}
}

