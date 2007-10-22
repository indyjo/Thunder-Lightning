#include <string>
#include <modules/clock/clock.h>
#include <modules/actors/fx/SpecialEffects.h>
#include <modules/gunsight/gunsight.h>
#include <sigc++/bind.h>
#include "tank.h"
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <interfaces/IModelMan.h>
#include <interfaces/ITerrain.h>
#include <sound.h>
#include <remap.h>
#include <SceneRenderPass.h>
#include <modules/actors/SimpleView.h>
#include <modules/model/SkeletonProvider.h>
#include <modules/ui/PanelRenderPass.h>
#include <modules/weaponsys/Armament.h>
#include <modules/weaponsys/Targeter.h>
#include <modules/weaponsys/Cannon.h>
#include <modules/weaponsys/ProjectileLauncher.h>
#include <modules/engines/ChasingEngine.h>


#define PI 3.14159265358979323846

#define RADIUS 6.0f


/*
struct TurretView: public SimpleView {
	Ptr<TankEngine> engine;
	
	TurretView(
		Ptr<IActor> subject,
		Ptr<TankEngine> engine,
		Ptr<IDrawable> gunsight=0)
	:	SimpleView(subject,gunsight),
		engine(engine)
	{ }
	
	virtual void getPositionAndOrientation(Vector*pos, Matrix3 *orient)
	{
	    Vector right,up,front;
	    engine->getOrientation(&up,&right,&front);
	    *orient =
	    	MatrixFromColumns(right,up,front)
	        * RotateYMatrix<float>(engine->getTurretAngle());
	    *pos = subject->getLocation() + (*orient) * Vector(0,6,-12);
	}
};
*/

Tank::Tank(Ptr<IGame> thegame, IoObject * io_peer_init)
: SimpleActor(thegame),
  renderer(thegame->getRenderer()),
  terrain(thegame->getTerrain()), damage(0),
  age(0)
{
    if (io_peer_init)
        setIoObject(io_peer_init);
    else
        createIoObject();
    
    setTargetInfo(new TargetInfo(
        "Tank", RADIUS, TargetInfo::TANK));

    tank_controls = new TankControls(getControls());
    setControlMode(AUTOMATIC);

    tank_engine = new TankEngine(thegame);
    setEngine(tank_engine);
    
    setTargeter(new Targeter(thegame->getTerrain(), *thegame, *this));

    //brain = new TankBrain(thegame, thegame->getClock(),
    //                      this, tank_controls, tank_engine);
    //brain->ballistic_cannon_control.onEnabled(*brain);
    //brain->maintain_position.onEnabled(*brain);
    //brain->move_to_exposed_point.onEnabled(*brain);

    Vector p = Vector(0, 0, 1000);
    Vector v = Vector(0, 0, 0);
    setLocation(p);
    setMovementVector(v);
    
    std::string skeletonfile = thegame->getConfig()->query("Tank_skeleton");
    setSkeleton(new Skeleton(thegame, skeletonfile));
    
    // Prepare collidable
    setBoundingGeometry(
        thegame->getCollisionMan()->queryGeometry(
            thegame->getConfig()->query("Tank_model_bounds")));
    // Don't set a rigid body, this is a static collidable!
    setActor(this);
    
    setArmament(new Armament(this, this));
    
    
    Ptr<IConfig> cfg = thegame->getConfig();
    Ptr<Cannon> machinegun = new Cannon(thegame, "Vulcan", cfg->queryInt("Tank_vulcan_rounds",1500));
    machinegun->factor = thegame->getConfig()->queryFloat("Tank_vulcan_factor", 1);
    machinegun->addBarrel(new SkeletonProvider(skeleton, "MGTopRight", "CannonTip", "CannonTipFront"));
    machinegun->addBarrel(new SkeletonProvider(skeleton, "MGBottomLeft", "CannonTip", "CannonTipFront"));
    machinegun->addBarrel(new SkeletonProvider(skeleton, "MGTopLeft", "CannonTip", "CannonTipFront"));
    machinegun->addBarrel(new SkeletonProvider(skeleton, "MGBottomRight", "CannonTip", "CannonTipFront"));
    machinegun->setLoadTime(
        thegame->getConfig()->queryFloat("Tank_vulcan_loadtime", machinegun->getLoadTime()));
    armament->addWeapon(0, machinegun);

    Ptr<Cannon> cannon=new Cannon(thegame, "Cannon", cfg->queryInt("Tank_cannon_rounds", 25));
    cannon->addBarrel(new SkeletonProvider(skeleton, "CannonTip", "CannonTip", "CannonTipFront"));
    cannon->factor = thegame->getConfig()->queryFloat("Tank_cannon_factor", 10);
    armament->addWeapon(0,cannon);
    
    sound_low = thegame->getSoundMan()->requestSource();
    sound_low->setPosition(p);
    sound_low->setLooping(true);
    //sound_low->setGain(0.0001);
    sound_low->setReferenceDistance(5);

    sound_high = thegame->getSoundMan()->requestSource();
    sound_high->setPosition(p);
    sound_high->setLooping(true);
    sound_high->setReferenceDistance(15);
    //sound_high->setGain(0.0001);

    mapArmamentEvents();
    mapTargeterEvents();
}

Tank::~Tank() {
}

void Tank::onLinked() {
    SimpleActor::onLinked();

    thegame->getCollisionMan()->add(this);
    
    sound_low->play(thegame->getSoundMan()->querySound(
            thegame->getConfig()->query("Tank_engine_sound_low")));
    sound_high->play(thegame->getSoundMan()->querySound(
            thegame->getConfig()->query("Tank_engine_sound_high")));
    
    updateDerivedObjects();
}

void Tank::onUnlinked() {
    SimpleActor::onUnlinked();

    thegame->getCollisionMan()->remove(this);

    sound_low->stop();
    sound_high->stop();
}

void Tank::action() {
    if (state == DEAD) return;
    float delta_t = thegame->getClock()->getStepDelta();

    age+=delta_t;

    Ptr<IActor> target = targeter->getCurrentTarget();

    if (control_mode == AUTOMATIC) {
        // script controlled, nothing to do here
    } else if (control_mode == MANUAL) {
    	Ptr<EventRemapper> remap = thegame->getEventRemapper();
    	tank_controls->setSteer(remap->getAxis("car_steer"));
    	tank_controls->setThrottle(remap->getAxis("car_throttle"));
    	tank_controls->setBrake(remap->getAxis("car_brake"));
    	tank_controls->setTurretSteer(remap->getAxis("turret_steer"));
    	tank_controls->setCannonSteer(remap->getAxis("cannon_steer"));
    }

    SimpleActor::action();
    
    updateDerivedObjects();
}

void Tank::draw() {
    renderer->enableLighting();
    SimpleActor::draw();
    renderer->disableLighting();
}

// Our tank has been hit ...
void Tank::applyDamage(float damage, int domain, Ptr<IProjectile> projectile) {
	if (projectile && projectile->getSource()) {
		Ptr<IActor> src = projectile->getSource();
		float dist2 = (src->getLocation()-getLocation()).lengthSquare();

		if (src->getFaction()->getAttitudeTowards(getFaction()) != Faction::FRIENDLY)
			targeter->setCurrentTarget(src);
	}
	damage *= 0.2;
    if (this->damage < 0.7 && this->damage+damage>0.7) {
        tankFirstExplosion(thegame, this);
    }
    this->damage += damage;
    if (this->damage > 1.0) explode();
}

void Tank::setLocation(const Vector & p) {
    SimpleActor::setLocation(p);
    //brain->move_to_exposed_point.onEnabled(*brain);
}

bool Tank::hasControlMode(ControlMode) {
  return true;
}
void Tank::setControlMode(ControlMode m) {
    SimpleActor::setControlMode(m);
    if (m==UNCONTROLLED) {
	    tank_controls->setSteer(0.0f);
	    tank_controls->setThrottle(0.0f);
	    tank_controls->setTurretSteer(0.0f);
	    tank_controls->setCannonSteer(0.0f);
    }
}

int Tank::getNumViews() {
	return 3;
}

Ptr<IView> Tank::getView(int n) {
    Ptr<SimpleActor> chaser = new SimpleActor(thegame);
    thegame->addWeakActor(chaser);
    
    Ptr<FlexibleGunsight> gunsight = new FlexibleGunsight(thegame);
	gunsight->addBasics(thegame, this);
    gunsight->addTargeting(chaser, targeter, armament);
    gunsight->addArmamentToScreen(thegame, armament, 0);

    Ptr<SceneRenderPass> scene_pass = thegame->createRenderPass(chaser);
    gunsight->setCamera(scene_pass->context.camera);
    
    Ptr<UI::PanelRenderPass> hud_pass = new UI::PanelRenderPass(thegame->getRenderer());
    hud_pass->stackedOn(scene_pass);
    hud_pass->setPanel(gunsight);
    
    Ptr<SimpleView> view = new SimpleView(this, chaser, hud_pass);
    mapViewEvents(view);
    
    switch(n) {
    case 0:
        chaser->setEngine(new ChasingEngine(thegame,this, 0.0f, 0.1f,
            Transform::identity(),
            Transform(Quaternion(1,0,0,0), Vector(0,4,-8))));
        break;
    case 1:
        gunsight->addBasicCrosshairs();
        chaser->setEngine(new ChasingEngine(thegame,new SkeletonProvider(skeleton, "MGCamera",
                "MG", "MGFront",
                "MG", "MGUp"), 0.0f, 0.1f));
        break;
    /*
    case 2:
    	view = new TurretView(this, tank_engine, gunsight1);
        chaser->setEngine(new ChasingEngine(thegame,view, 0.1f, 0.1f));
        view->setGunsight(gunsight1);
        break;
    */
    case 2:
        chaser->setEngine(new ChasingEngine(thegame, this, 0.1f, 0.5f,
            Transform::identity(),
            Transform(Quaternion::Rotation(Vector(0,1,0), PI), Vector(0,6,18))));
        break;
    default:
    	return 0;
    }

    return view;
}

void Tank::explode() {
    kill();
    tankFinalExplosion(thegame, this);
    sound_high->stop();
    sound_low->stop();
}

void Tank::integrate(float delta_t, Transform * transforms) {
    // as this is a "static" actor, we only encode the current state
    transforms[0] = skeleton->getRootBoneTransform();
    transforms[1] = skeleton->getEffectiveBoneTransform("Turret");
    transforms[2] = skeleton->getEffectiveBoneTransform("Cannon");
    transforms[3] = skeleton->getEffectiveBoneTransform("MachineGun");
}

void Tank::update(float delta_t, const Transform * new_transforms) {
    // as this is a "static" actor, we can ignore this
}

void Tank::updateDerivedObjects() {
    skeleton->setBoneTransform("Turret", Transform(
        Quaternion::Rotation(Vector(0,1,0), tank_engine->getTurretAngle()), Vector(0,0,0)));
    skeleton->setBoneTransform("Cannon", Transform(
        Quaternion::Rotation(Vector(-1,0,0), tank_engine->getCannonAngle()), Vector(0,0,0)));
    skeleton->setBoneTransform("MachineGun", Transform(
        Quaternion::Rotation(Vector(-1,0,0), tank_engine->getCannonAngle()), Vector(0,0,0)));
    
    sound_high->setPosition(getLocation());
    sound_high->setVelocity(getMovementVector());
    sound_low->setPosition(getLocation());
    sound_low->setVelocity(getMovementVector());

    float v = getMovementVector().length();
    //float gain = 1.0f - std::min(1.0f, v/20.0f);
    //float gain = 1.0f;
    //gain *= gain;
    //gain *= gain;
    sound_high->setPitch(0.5f + 1.5*std::min(1.0f,v/30));
    
    float gain = std::min(v, 2.0f)/2.0f;
    sound_high->setGain(gain);
    sound_low->setGain(1-gain);
}

