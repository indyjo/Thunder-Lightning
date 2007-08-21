#include <string>
#include <interfaces/IConfig.h>
#include <interfaces/IModelMan.h>
#include <modules/actors/SimpleView.h>
#include <modules/camera/FollowingCamera.h>
#include <modules/collide/CollisionManager.h>
#include <modules/engines/ChasingEngine.h>
#include <modules/engines/effectors.h>
#include <modules/engines/Turret.h>
#include <modules/gunsight/gunsight.h>
#include <modules/model/SkeletonProvider.h>
#include <modules/ui/PanelRenderPass.h>
#include <modules/weaponsys/Armament.h>
#include <modules/weaponsys/Targeter.h>
#include <modules/weaponsys/Cannon.h>
#include <remap.h>
#include <SceneRenderPass.h>
#include "Carrier.h"

Carrier::Carrier(Ptr<IGame> thegame, IoObject * io_peer_init)
    : SimpleActor(thegame)
    , control_target(CARRIER)
{
    if (io_peer_init)
        setIoObject(io_peer_init);
    else {
        //createIoObject();
    }
    
    setTargetInfo(new TargetInfo(
        "Carrier", 78, TargetInfo::CARRIER));
        
    this->engine = new RigidEngine(thegame);
    float m = thegame->getConfig()->queryFloat("Carrier_mass", 50000000.0f); // mass
    float w = 2*30.96f;    // width, height and depth taken from the model
    float h = 2*14.833f;
    float d = 2*76.524f;
    float f = m / 12;
    // construct intertia like cuboid with these dimensions
    engine->construct(m, f*(h*h+d*d), f*(w*w+d*d), f*(w*w+h*h));
    engine->addEffector(Effectors::Gravity::getInstance());
    setEngine(engine);
    
    std::string skeletonfile = thegame->getConfig()->query("Carrier_skeleton");
    setSkeleton(new Skeleton(thegame, skeletonfile));

    Ptr<Model> hull_model = thegame->getModelMan()->query(thegame->getConfig()->query("Carrier_model_hull"));
    Effectors::Buoyancy::addBuoyancyFromMesh(
        engine,
        hull_model->getDefaultObject(),
        Vector(0,0,0));
        
    
    // Prepare collidable
    setBoundingGeometry(
        thegame->getCollisionMan()->queryGeometry(
            thegame->getConfig()->query("Carrier_model_bounds")));
    setRigidBody(ptr(engine));
    setActor(this);
    
    Transform xform(
        Quaternion::Rotation(Vector(-1,0,0), 70*3.141593f/180), 
        Vector(0,0,0));
    skeleton->setBoneTransform("BayDoor", xform);
    
    TurretElement base, elevator;
    base.axis = Vector(0,1,0);
    base.reference = Vector(0,0,1);
    base.min_angle = base.max_angle = 0;
    base.is_restricted = false;
    base.max_rotation_speed = 1.0f;
    elevator.axis = Vector(-1,0,0);
    elevator.reference = Vector(0,0,1);
    elevator.min_angle = -15*3.141592f/180;
    elevator.max_angle = 90*3.141592f/180;
    elevator.is_restricted = true;
    elevator.max_rotation_speed = 1.0f;
    Vector pivot(0,0,0);
    main_turret = new Turret(base, elevator, pivot);
    
    setTargeter(new Targeter(thegame->getTerrain(), *thegame, *this));
    setArmament(new Armament(this, this));
    
    Ptr<IConfig> cfg = thegame->getConfig();
    
    Ptr<Cannon> machinegun = new Cannon(thegame,
        "Vulcan", cfg->queryInt("Carrier_Vulcan_rounds", 1500));
    machinegun->factor = thegame->getConfig()->queryFloat("Carrier_Vulcan_factor", 1);
    machinegun->addBarrel(new SkeletonProvider(skeleton, "main_turret_nozzle_left", "main_turret_pivot", "main_turret_pivot_dir"));
    machinegun->addBarrel(new SkeletonProvider(skeleton, "main_turret_nozzle_right", "main_turret_pivot", "main_turret_pivot_dir"));
    armament->addWeapon(0, machinegun);
    
    mapArmamentEvents();
    mapTargeterEvents();

}

void Carrier::onLinked() {
    SimpleActor::onLinked();
    thegame->getCollisionMan()->add(this);
    updateDerivedObjects();
}

void Carrier::onUnlinked() {
    SimpleActor::onUnlinked();
    thegame->getCollisionMan()->remove(this);
}

void Carrier::action() {
	if (!isAlive()) return;
	SimpleActor::action();
    
    if (control_mode == MANUAL) {
        Ptr<EventRemapper> remap = thegame->getEventRemapper();
        switch(control_target) {
        case CARRIER:
            break;
        case MAIN_TURRET:
            controls->setFloat("main_turret_steer_x", remap->getAxis("tank_turret_steer"));
            controls->setFloat("main_turret_steer_y", remap->getAxis("tank_cannon_steer"));
            break;
        }
    }
    
    controls->setFloat("main_turret_angle_x", main_turret->getAngle(0));
    controls->setFloat("main_turret_angle_y", main_turret->getAngle(1));
}

bool Carrier::hasControlMode(ControlMode) {
  return true;
}

void Carrier::setControlMode(ControlMode m) {
    SimpleActor::setControlMode(m);
    if (m==UNCONTROLLED) {
        controls->setFloat("main_turret_steer_x", 0);
        controls->setFloat("main_turret_steer_y", 0);
    }
}

int Carrier::getNumViews() {
	return 6;
}

Ptr<IView> Carrier::getView(int n) {
    Ptr<SimpleActor> chaser = new SimpleActor(thegame);
    
    Ptr<FlexibleGunsight> gunsight = new FlexibleGunsight(thegame);
	gunsight->addBasics(thegame, this);

    Ptr<SceneRenderPass> scene_pass = thegame->createRenderPass(chaser);
    gunsight->setCamera(scene_pass->context.camera);
    
    Ptr<UI::PanelRenderPass> hud_pass = new UI::PanelRenderPass(thegame->getRenderer());
    hud_pass->stackedOn(scene_pass);
    hud_pass->setPanel(gunsight);
    
    Ptr<SimpleView> view = new SimpleView(this, chaser, hud_pass);
    mapViewEvents(view);
    
    switch(n) {
    case 0:
        chaser->setEngine(new ChasingEngine(thegame,new SkeletonProvider(skeleton,
                "frontview_viewpoint",
                "frontview_viewpoint", "frontview_lookat",
                "origin", "up"),
                0.1f, 0.1f));
        view->onEnable().connect(SigC::bind(
            SigC::slot(*this, & Carrier::setControlTarget),
            CARRIER));
        break;
    case 1:
        gunsight->addBasicCrosshairs();
        gunsight->addTargeting(chaser, targeter, armament);
        gunsight->addArmamentToScreen(thegame, armament, 0);
        chaser->setEngine(new ChasingEngine(thegame,new SkeletonProvider(skeleton,
                "main_turret_viewpoint",
                "main_turret_pivot", "main_turret_pivot_dir",
                "main_turret_pivot", "main_turret_pivot_up"),
                0.0f, 0.1f));
        view->onEnable().connect(SigC::bind(
            SigC::slot(*this, & Carrier::setControlTarget),
            MAIN_TURRET));
        break;
    case 2:
        chaser->setEngine(new ChasingEngine(thegame,new SkeletonProvider(skeleton,
                "backview_viewpoint",
                "backview_viewpoint", "backview_lookat",
                "origin", "up"),
                0.1f, 0.1f));
        view->onEnable().connect(SigC::bind(
            SigC::slot(*this, & Carrier::setControlTarget),
            CARRIER));
        break;
    case 3:
        chaser->setEngine(new ChasingEngine(thegame,new SkeletonProvider(skeleton,
                "deckview_viewpoint",
                "deckview_viewpoint", "deckview_lookat",
                "origin", "up"),
                0.1f, 0.1f));
        view->onEnable().connect(SigC::bind(
            SigC::slot(*this, & Carrier::setControlTarget),
            CARRIER));
        break;
    case 4:
        chaser->setEngine(new ChasingEngine(thegame,new SkeletonProvider(skeleton,
                "landingview_viewpoint",
                "landingview_viewpoint", "landingview_lookat",
                "origin", "up"),
                0.1f, 0.1f));
        view->onEnable().connect(SigC::bind(
            SigC::slot(*this, & Carrier::setControlTarget),
            CARRIER));
        break;
    case 5:
        chaser->setEngine(new ChasingEngine(thegame,new SkeletonProvider(skeleton,
                "bayview_viewpoint",
                "bayview_viewpoint", "bayview_lookat",
                "origin", "up"),
                0.1f, 0.1f));
        view->onEnable().connect(SigC::bind(
            SigC::slot(*this, & Carrier::setControlTarget),
            CARRIER));
        break;
    default:
    	return 0;
    }

    thegame->addWeakActor(chaser);
    return view;
}

void Carrier::integrate(float delta_t, Transform * transforms) {
    engine->integrate(delta_t, transforms);
    transforms[1] = skeleton->getEffectiveBoneTransform("Deck");
    transforms[2] = skeleton->getEffectiveBoneTransform("DeckTower");
    transforms[3] = skeleton->getEffectiveBoneTransform("Radom");
    transforms[4] = skeleton->getEffectiveBoneTransform("DockingBay");
    transforms[5] = skeleton->getEffectiveBoneTransform("BayDoor");
    transforms[6] = main_turret->integrate(0, delta_t, controls->getFloat("main_turret_steer_x"));
    transforms[7] = main_turret->integrate(1, delta_t, controls->getFloat("main_turret_steer_y"));
}


void Carrier::update(float delta_t, const Transform * new_transforms) {
    engine->update(delta_t, new_transforms);
    main_turret->update(0, new_transforms[6]);
    main_turret->update(1, new_transforms[7]);
    skeleton->setBoneTransform("Turret", main_turret->integrate(0,0,0));
    skeleton->setBoneTransform("Cannon", main_turret->integrate(1,0,0));
}


void Carrier::setControlTarget(ControlTarget c) {
    control_target = c;
    ls_message("Control target: %d\n", c);
}

void Carrier::updateDerivedObjects() {
}


