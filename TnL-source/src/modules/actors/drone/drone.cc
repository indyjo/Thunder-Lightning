#include <cstdlib>
#include <iostream>
#include <string>
#include <sigc++/bind.h>
#include <modules/actors/fx/SpecialEffects.h>
#include <modules/camera/FollowingCamera.h>
#include <modules/clock/clock.h>
#include <modules/engines/effectors.h>
#include <modules/engines/rigidengine.h>
#include <modules/environment/Water.h>
#include <modules/gunsight/gunsight.h>
#include <modules/math/SpecialMatrices.h>
#include <modules/model/Skeleton.h>
#include <modules/model/SkeletonProvider.h>
#include <modules/ui/PanelRenderPass.h>
#include <modules/weaponsys/Targeter.h>
#include <modules/weaponsys/Cannon.h>
#include <modules/weaponsys/ProjectileLauncher.h>
#include <modules/scripting/IoScriptingManager.h>
#include <modules/scripting/mappings.h>
#include <modules/engines/ChasingEngine.h>

#include <remap.h>
#include <sound.h>
#include <SceneRenderPass.h>
#include <interfaces/IConfig.h>
#include <interfaces/ICamera.h>
#include <interfaces/IModelMan.h>
#include <interfaces/ITerrain.h>

#include "drone.h"
#include "DroneCockpit.h"

#include <modules/actors/SimpleView.h>

#define PI 3.14159265358979323846

#define RADIUS 10.0f

#define CURRENT_IDEA_BONUS 0.05f

/*
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
*/


Drone::Drone(Ptr<IGame> thegame
#ifdef HAVE_IO
    , IoObject* io_peer_init
#endif
)
: SimpleActor(thegame),
  renderer(thegame->getRenderer()),
  terrain(thegame->getTerrain()), damage(0),
  gear_lowered(false),
  hook_lowered(false),
  engine_power(1.0)
{
	ls_message("<Drone::Drone>\n");
#ifdef HAVE_IO
    if (io_peer_init)
        setIoObject(io_peer_init);
    else
        createIoObject();
#endif    		
	ls_message("</Drone::Drone>\n");
}

void Drone::init() {
	ls_message("<Drone::init>\n");
	
	Ptr<IConfig> cfg = thegame->getConfig();
    setTargetInfo(new TargetInfo(
        "Drone", cfg->queryFloat("Drone_target_radius",15),
        TargetInfo::AIRCRAFT));

    flight_controls = new FlightControls(controls);
    engine = new RigidEngine(thegame);
    engine->construct(2000, 200000, 160000, 100000);
    engine->addEffector( new Effectors::Flight(flight_controls) );
    engine->addEffector( Effectors::Gravity::getInstance() );
    
    setEngine(engine);
    
    flight_controls->setThrottle(1.0f);

    // Load drone model and skeleton, even if there are no moving parts
    std::string skeletonfile = thegame->getConfig()->query("Drone_skeleton");
    skeleton = new Skeleton(thegame, skeletonfile);

    // Weapons stuff
    setArmament(new Armament(this, this));
    targeter = new Targeter(thegame->getTerrain(), *thegame,*this);

    Ptr<Cannon> machinegun = new Cannon(thegame, "Vulcan",
        cfg->queryInt("Drone_Vulcan_rounds", 500));
    machinegun->addBarrel(new SkeletonProvider(skeleton, "MG_0", "origin", "z"));
    machinegun->addBarrel(new SkeletonProvider(skeleton, "MG_1", "origin", "z"));
    armament->addWeapon(0,machinegun);

    Ptr<ProjectileLauncher> smart_missile_launcher = new ProjectileLauncher(
        thegame, targeter, ProjectileFactories::factories.smart_missile_2_factory,
        "Sidewinder",
        cfg->queryInt("Drone_Sidewinder_rounds", 6));
    smart_missile_launcher->addBarrel(
        new SkeletonProvider(skeleton, "Launcher_0", "origin", "z"));
    smart_missile_launcher->addBarrel(
        new SkeletonProvider(skeleton, "Launcher_1", "origin", "z"));
    armament->addWeapon(0,smart_missile_launcher);

    Ptr<ProjectileLauncher> dumb_missile_launcher = new ProjectileLauncher(
        thegame, targeter, ProjectileFactories::factories.dumb_missile_factory,
        "Hydra",
        cfg->queryInt("Drone_Hydra_rounds", 14));
    dumb_missile_launcher->addBarrel(
        new SkeletonProvider(skeleton, "Launcher_0", "origin", "z"));
    dumb_missile_launcher->addBarrel(
        new SkeletonProvider(skeleton, "Launcher_1", "origin", "z"));
    armament->addWeapon(0,dumb_missile_launcher);

    Ptr<ProjectileLauncher> decoy_launcher = new ProjectileLauncher(
        thegame, targeter, ProjectileFactories::factories.decoy_factory,
        "Decoy",
        cfg->queryInt("Drone_Decoy_rounds", 14));
    decoy_launcher->addBarrel(
        new SkeletonProvider(skeleton, "Launcher_0", "origin", "z"));
    decoy_launcher->addBarrel(
        new SkeletonProvider(skeleton, "Launcher_1", "origin", "z"));
    armament->addWeapon(0,decoy_launcher);

    // Prepare collidable
    Ptr<Collide::BoundingGeometry> bounds =
        thegame->getCollisionMan()->queryGeometry(cfg->query("Drone_model_bounds"));
    if (!bounds) {
        throw std::runtime_error("Bounds are null");
    } else {
        ls_message("Drone: setting bounds to %p\n", &*bounds);
    }
    setBoundingGeometry(bounds);
    setRigidBody(&*engine);
    setActor(this);
    
    // Tail hook
    hook = new Effectors::TailHook(
        thegame->getCollisionMan(),
        this,
        skeleton->getUntransformedPoint("TailHook_p0"),
        skeleton->getUntransformedPoint("TailHook_p1"),
        thegame->getConfig()->queryFloat("Drone_max_tailhook_force", 80000)
    );

	// Wheels
	Effectors::Wheel::Params nose_wheel = {
	    skeleton->getUntransformedPoint("NoseWheel"),               // position
	    Vector(1,0,0),                                              // axis
	    thegame->getConfig()->queryFloat("Drone_NoseWheel_range", 1), // range
	    thegame->getConfig()->
	        queryFloat("Drone_NoseWheel_force", 50000),               // force
	    thegame->getConfig()->
	        queryFloat("Drone_NoseWheel_damping", 15000),             // damping
	    thegame->getConfig()->
	        queryFloat("Drone_NoseWheel_C_tan_min", 50),              // tangential resistance
	    thegame->getConfig()->
	        queryFloat("Drone_NoseWheel_C_norm", 10000)};             // normal resistance

	Effectors::Wheel::Params left_wheel = {
	    skeleton->getUntransformedPoint("LeftWheel"),               // position
	    Vector(1,0,0),                                              // axis
	    thegame->getConfig()->queryFloat("Drone_LeftWheel_range", 1), // range
	    thegame->getConfig()->
	        queryFloat("Drone_LeftWheel_force", 50000),               // force
	    thegame->getConfig()->
	        queryFloat("Drone_LeftWheel_damping", 15000),             // damping
	    thegame->getConfig()->
	        queryFloat("Drone_LeftWheel_C_tan_min", 50),              // tangential resistance
	    thegame->getConfig()->
	        queryFloat("Drone_LeftWheel_C_norm", 1000)};             // normal resistance
	        
	Effectors::Wheel::Params right_wheel = {
	    skeleton->getUntransformedPoint("RightWheel"),               // position
	    Vector(1,0,0),                                              // axis
	    thegame->getConfig()->queryFloat("Drone_RightWheel_range", 1), // range
	    thegame->getConfig()->
	        queryFloat("Drone_RightWheel_force", 50000),               // force
	    thegame->getConfig()->
	        queryFloat("Drone_RightWheel_damping", 15000),             // damping
	    thegame->getConfig()->
	        queryFloat("Drone_RightWheel_C_tan_min", 50),              // tangential resistance
	    thegame->getConfig()->
	        queryFloat("Drone_RightWheel_C_norm", 1000)};             // normal resistance
	
    wheels[0] = new Effectors::Wheel(terrain, thegame->getCollisionMan(), this, nose_wheel);
    wheels[1] = new Effectors::Wheel(terrain, thegame->getCollisionMan(), this, left_wheel);
    wheels[2] = new Effectors::Wheel(terrain, thegame->getCollisionMan(), this, right_wheel);

    std::string model_file = cfg->query("Drone_wheel_model_file");
    wheel_model = thegame->getModelMan()->query(model_file);
    
    // Cockpit initialization ...

    std::string inside_model_file = cfg->query("Drone_inside_model_file");
    Ptr<Model> inside_model = thegame->getModelMan()->query(inside_model_file);
    
    std::string mfd_model_file = cfg->query("Drone_mfd_model_file");
    Ptr<Model> mfd_model = new Model(*thegame->getTexMan(), mfd_model_file);
    
    this->cockpit = new DroneCockpit(thegame, thegame->getRenderer(), this, inside_model, mfd_model);

    setControlMode(UNCONTROLLED);

    damage = 0;

    mapTargeterEvents();
    mapArmamentEvents();
    
    Ptr<EventSheet> sheet = getEventSheet();
    sheet->map("landing-gear", SigC::slot(*this, &Drone::toggleLandingGear));
    sheet->map("landing-hook", SigC::slot(*this, &Drone::toggleLandingHook));
    
    sheet->map("switch-mfd", SigC::slot(*cockpit, &DroneCockpit::switchMfdMode));
    
	ls_message("</Drone::init>\n");
}

Drone::~Drone() {
	ls_message("<Drone::~Drone()>\n");
	ls_message("</Drone::~Drone()>\n");
}

void Drone::onLinked() {
    ls_message("Drone linked\n");
    SimpleActor::onLinked();
    thegame->getCollisionMan()->add(this);
    
    engine_sound_src = thegame->getSoundMan()->requestSource();
    engine_sound_src->setPosition(getLocation());
    engine_sound_src->setVelocity(getMovementVector());
    engine_sound_src->setLooping(true);
    engine_sound_src->setMaxGain(thegame->getConfig()->queryFloat("Drone_engine_max_gain", 1.0f));
    engine_sound_src->setReferenceDistance(thegame->getConfig()->queryFloat("Drone_engine_reference_distance", 1.0f));
    engine_sound_src->play(thegame->getSoundMan()->querySound(
            thegame->getConfig()->query("Drone_engine_sound")));

    skeleton->setBoneTransform("Body",getTransform());
    updateDerivedObjects();
    ls_message("Done: Drone linked\n");
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
        // Drone is script-controlled now, nothing to do here.
    } else if (control_mode == MANUAL) {
        flight_controls->setRudder(    thegame->getEventRemapper()->getAxis("rudder") );
        flight_controls->setAileron(   thegame->getEventRemapper()->getAxis("aileron") );
        flight_controls->setElevator( -thegame->getEventRemapper()->getAxis("elevator") );
        flight_controls->setThrottle(  thegame->getEventRemapper()->getAxis("throttle") );
        flight_controls->setBrake( thegame->getEventRemapper()->getAxis("brake") );
    }
    
    setLandingGear(flight_controls->isGearLowered());
    setLandingHook(flight_controls->isHookLowered());

    // set brake factors on main landing gear, 0 <= u <= 1
    float u = controls->getFloat("brake",0);
    wheels[0]->getParams().drag_long =
        (1-u) * thegame->getConfig()->queryFloat("Drone_NoseWheel_C_tan_min", 50)
          + u * thegame->getConfig()->queryFloat("Drone_NoseWheel_C_tan_max", 650);
    wheels[1]->getParams().drag_long =
        (1-u) * thegame->getConfig()->queryFloat("Drone_LeftWheel_C_tan_min", 50)
          + u * thegame->getConfig()->queryFloat("Drone_LeftWheel_C_tan_max", 1000);
    wheels[2]->getParams().drag_long =
        (1-u) * thegame->getConfig()->queryFloat("Drone_RightWheel_C_tan_min", 50)
          + u * thegame->getConfig()->queryFloat("Drone_RightWheel_C_tan_max", 1000);
    
    // rotate the front wheel according to rudder
    static const float max_wheel_turn = 60*3.141593f/180; // 60 degrees max
    wheels[0]->getParams().axis = Vector(
        cos(max_wheel_turn * controls->getFloat("rudder")),
        sin(max_wheel_turn * controls->getFloat("rudder")),
        0);
        
    // cheap-ass crash detection
    Vector p = getLocation();
    if (p[1] < 0) {
        p[1] = 0;
        setLocation(p);
        explode();
    }
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
    
    updateDerivedObjects();
}

void Drone::kill() {
	targeter->clearCurrentTarget();
    setCollidingEnabled(false);
    SimpleActor::kill();
}

#define MAX_MODEL_DISTANCE 3000.0f
#define MAX_POINT_DISTANCE 10000.0f

void Drone::draw() {
	if (!isAlive()) return;
    SimpleActor::draw();
    
    if (isLandingGearLowered()) {
        drawWheels();
    }
}

// Our drone has been hit ...
void Drone::applyDamage(float damage, int domain, Ptr<IProjectile> projectile) {
	if (!isAlive()) return;
    SimpleActor::applyDamage(damage,domain,projectile);

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
	return 5;
}

Ptr<IView> Drone::getView(int n) {
	Vector pilot_pos = thegame->getConfig()->queryVector(
		"Drone_pilot_pos", Vector(0,0,0));
	
    Ptr<SimpleActor> chaser = new SimpleActor(thegame);
    thegame->addWeakActor(chaser);
    
    Ptr<FlexibleGunsight> gunsight = new FlexibleGunsight(thegame);
	gunsight->addBasics(thegame, this);
	gunsight->addTargeting(chaser, targeter, armament);
	gunsight->addDirectionOfFlight(this);
    gunsight->addArmamentToScreen(thegame, armament, 0);
    gunsight->addMissileWarning(thegame, this);

    Ptr<SceneRenderPass> scene_pass = thegame->createRenderPass(chaser);
    gunsight->setCamera(scene_pass->context.camera);
    Ptr<RenderPass> cockpit_pass;
    
    Ptr<UI::PanelRenderPass> hud_pass = new UI::PanelRenderPass(thegame->getRenderer());
    hud_pass->setPanel(gunsight);
    
    // For inside-cockpit view: setup cockpit rendering
    if (n == 0 || n == 2 || n == 3 || n == 5) {
        Ptr<FollowingCamera> cam = new FollowingCamera;
        cam->setTarget(scene_pass->context.camera);
        cam->ownAspect(false);
        cam->ownFocus(false);
        cam->setNearDistance(0.1);
        cam->setFarDistance(4.0);

        cockpit_pass = cockpit->createRenderPass(cam);
        cockpit_pass->getBottomOfStack()->stackedOn(scene_pass);
    
        hud_pass->stackedOn(cockpit_pass);
    } else {
        hud_pass->stackedOn(scene_pass);
    }
    
    Ptr<SimpleView> view = new SimpleView(this, chaser, hud_pass);
    view->onEnable().connect(
        SigC::slot(*gunsight, &UI::Panel::enable));
    view->onDisable().connect(
        SigC::slot(*gunsight, &UI::Panel::disable));
    mapViewEvents(view);
    
    
    Transform xform(Quaternion(1,0,0,0), pilot_pos);

    // For now, use flight modules in every view
    gunsight->addFlightModules(thegame, flight_info, controls);
	switch(n) {
    case 0:
	    gunsight->addBasicCrosshairs();
    	//gunsight->addFlightModules(thegame, flight_info, controls);
        chaser->setEngine(new ChasingEngine(thegame,this, 0.0f, 0.1f,
            xform,
            Transform::identity() ));
    	break;
    case 1:
        chaser->setEngine(new ChasingEngine(thegame,this, 0.0f, 0.1f,
            xform,
            Transform(Quaternion::Rotation(Vector(0,1,0), 180*PI/180), Vector(0,2,12)) ));
    	break;
    case 2:
        chaser->setEngine(new ChasingEngine(thegame,this, 0.0f, 0.1f,
            xform,
            Transform(Quaternion::Rotation(Vector(0,1,0), 90*PI/180), Vector(0)) ));
    	break;
    case 3:
        chaser->setEngine(new ChasingEngine(thegame,this, 0.0f, 0.1f,
            xform,
            Transform(Quaternion::Rotation(Vector(0,1,0), -90*PI/180), Vector(0)) ));
    	break;
    case 4:
        chaser->setEngine(new ChasingEngine(thegame,this, 0.2f, 0.5f,
            Transform(Quaternion(1,0,0,0), Vector(0,3,-8)),
            Transform::identity() ));
    	break;
    case 5:
        /*
        {
            Ptr<TargetView> target_view = new TargetView(this, pilot_pos, targeter);
            Transform xform(Quaternion(1,0,0,0), Vector(0,0,0));
            chaser->setEngine(new ChasingEngine(thegame, target_view, 0.0f, 0.1f, xform));
            break;
        }
        */
    default:
    	return 0;
	}
	
	return view;
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

void Drone::collide(const Collide::Contact & c) {
    Ptr<Collidable> partner = c.collidables[1];
    
    float impulse = 0;
    Ptr<RigidBody> rigid = partner->getRigid();
    if (rigid) {
        impulse = RigidBody::collisionImpulseMagnitude(1.0, *engine, *rigid, c.p, c.n);
    } else {
        impulse = RigidBody::collisionImpulseMagnitude(1.0, *engine, c.p, c.n);
    }
    
    float drone_mass = engine->getBase().M;
    if (std::abs(impulse) > 8*drone_mass) {
        explode(true);
    }
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

void Drone::setLandingHook(bool lowered) {
    if (lowered == hook_lowered) {
        return;
    }
    
    hook_lowered=lowered;
    if (lowered) {
        hook->clear();
        engine->addEffector( hook );
    } else {
        engine->removeEffector( hook );
    }
}

void Drone::updateDerivedObjects() {
    float delta_t = thegame->getClock()->getFrameDelta();
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
    engine_sound_src->setMaxGain(thegame->getConfig()->queryFloat("Drone_engine_max_gain", 1.0f));
}

void Drone::drawWheels() {
    renderer->setCullMode(JR_CULLMODE_CULL_NEGATIVE);
    renderer->setAlpha(1);
    renderer->setColor(Vector(1,0,1));
    
    Matrix3 orient = getOrientationAsMatrix();
    
    renderer->enableLighting();
	for(int i=0; i<3; ++i) {
    	wheel_model->draw(*renderer, orient, wheels[i]->getCurrentPos());
	}
    renderer->disableLighting();
}

