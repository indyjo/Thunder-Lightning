#include <string>
#include <modules/clock/clock.h>
#include <modules/actors/fx/SpecialEffects.h>
#include <modules/actors/projectiles/bullet.h>
#include <modules/actors/fx/smoketrail.h>
#include <modules/gunsight/gunsight.h>
#include <sigc++/bind.h>
#include "tank.h"
#include "ai.h"
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <interfaces/IMap.h>
#include <interfaces/IModelMan.h>
#include <interfaces/ITerrain.h>
#include <sound.h>
#include <remap.h>
#include <modules/actors/SimpleView.h>
#include <modules/actors/RelativeView.h>
#include <modules/weaponsys/Armament.h>
#include <modules/weaponsys/Targeter.h>
#include <modules/weaponsys/Cannon.h>


#define PI 3.14159265358979323846

#define RADIUS 6.0f

#define RAND ((float) rand() / (float) RAND_MAX)
#define RAND2 ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)

#define MUZZLE_VELOCITY 1700.0f
#define BULLET_RANGE 10000.0f
#define BULLET_TTL (BULLET_RANGE / MUZZLE_VELOCITY)


struct PointView: public SimpleView {
    Ptr<Skeleton> skeleton;
    const char *pos, *vec1, *vec2;
    
    PointView(Ptr<IActor> subject, Ptr<Skeleton> skeleton,
        const char *pos, const char *vec1, const char *vec2,
        Ptr<IDrawable> gunsight=0)
    : SimpleView(subject, gunsight), skeleton(skeleton), pos(pos), vec1(vec1), vec2(vec2)
    { }
    
	virtual void getPositionAndOrientation(Vector *location, Matrix3 *orient)
	{
	    Vector p = skeleton->getPoint(pos);
	    Vector front = skeleton->getPoint(vec2)-skeleton->getPoint(vec1);
	    Vector right = Vector(0,1,0) % front;
	    Vector up = front % right;
	    
	    front.normalize();
	    right.normalize();
	    up.normalize();
	    
	    *orient = MatrixFromColumns(right,up,front);
	    *location = p;
	}
};    

struct CannonView: public SimpleView {
	Ptr<TankEngine> engine;
	
	CannonView::CannonView(
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
	        * RotateYMatrix<float>(engine->getTurretAngle())
	    	* RotateXMatrix<float>(-engine->getCannonAngle());
	    *pos = subject->getLocation() + (*orient) * Vector(0,6,-3);
	}
};

struct TurretView: public SimpleView {
	Ptr<TankEngine> engine;
	
	TurretView::TurretView(
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


struct SkeletonProvider : public IPositionProvider {
    Ptr<Skeleton> skeleton;
    const char * pos, *front1, *front2, *up1, *up2;
    
    SkeletonProvider(Ptr<Skeleton> skel, const char *p, const char *f1, const char *f2,
        const char* u1=0, const char *u2=0)
    :   skeleton(skel), pos(p), front1(f1), front2(f2), up1(u1), up2(u2)
    { }
    
    virtual Vector getLocation() { return skeleton->getPoint(pos); }
    Matrix3 orient() {
        Vector front = skeleton->getPoint(front2)-skeleton->getPoint(front1);
        Vector right, up;
        if (up1) {
            up = skeleton->getPoint(up2)-skeleton->getPoint(up1);
            right = up % front;
        } else {
            right = Vector(0,1,0) % front;
            right.normalize();
            up = front % right;
        }
        return MatrixFromColumns(right,up,front);
    }
    virtual Vector getFrontVector() {
        return skeleton->getPoint(front2)-skeleton->getPoint(front1);
    }
    virtual Vector getRightVector() {
        return orient()*Vector(1,0,0);
    }
    virtual Vector getUpVector() {
        return orient()*Vector(0,1,0);
    }
    virtual void getOrientation(Vector *up, Vector *right, Vector *front) {
        Matrix3 M = orient();
        *up = M * Vector(0,1,0);
        *right = M * Vector(1,0,0);
        *front = M * Vector(0,0,1);
    }
};

struct FollowingView : public SimpleView {
    Ptr<IPositionProvider> pp;
    
    inline FollowingView(Ptr<IPositionProvider> pp, Ptr<IActor> subject=0, Ptr<IDrawable> gunsight=0)
    : SimpleView(subject, gunsight), pp(pp)
    { }
    
	virtual void getPositionAndOrientation(Vector*pos, Matrix3 *orient)
	{
	    Vector right,up,front;
	    *pos = pp->getLocation();
	    pp->getOrientation(&up,&right,&front);
	    *orient = MatrixFromColumns(right,up,front);
	}
};

Tank::Tank(Ptr<IGame> thegame)
: SimpleActor(thegame),
  renderer(thegame->getRenderer()),
  terrain(thegame->getTerrain()), damage(0),
  age(0), control_mode(UNCONTROLLED),
  armament(this, 0)
{
    setTargetInfo(new TargetInfo(
        "Tank", RADIUS, TargetInfo::TANK));

    tank_controls = new TankControls();
    setControlMode(AUTOMATIC);

    tank_engine = new TankEngine(thegame, tank_controls);
    tank_engine->getFireSignal().connect(
        SigC::slot(*this, &Tank::shoot));
    setEngine(tank_engine);
    
    targeter = new Targeter(*thegame, *this);

    brain = new TankBrain(thegame, thegame->getClock(),
                          this, tank_controls, tank_engine);
    //brain->ballistic_cannon_control.onEnabled(*brain);
    //brain->maintain_position.onEnabled(*brain);
    brain->move_to_exposed_point.onEnabled(*brain);

    Vector p = Vector(0, 0, 1000);
    Vector v = Vector(0, 0, 0);
    setLocation(p);
    setMovementVector(v);
    
    std::string skeletonfile = thegame->getConfig()->query("Tank_skeleton");
    skeleton = new Skeleton(thegame, skeletonfile);
    
    Ptr<Cannon> cannon=new Cannon(thegame);
    cannon->addBarrel(new SkeletonProvider(skeleton, "CannonTip", "CannonTip", "CannonTipFront"));
    
    Ptr<Cannon> machinegun = new Cannon(thegame, "4x Vulcan", 1500, 4.0/60, false);
    machinegun->addBarrel(new SkeletonProvider(skeleton, "MGTopRight", "CannonTip", "CannonTipFront"));
    machinegun->addBarrel(new SkeletonProvider(skeleton, "MGBottomLeft", "CannonTip", "CannonTipFront"));
    machinegun->addBarrel(new SkeletonProvider(skeleton, "MGTopLeft", "CannonTip", "CannonTipFront"));
    machinegun->addBarrel(new SkeletonProvider(skeleton, "MGBottomRight", "CannonTip", "CannonTipFront"));
    
    armament.addWeapon(cannon);
    armament.addWeapon(machinegun);

    sound_low = thegame->getSoundMan()->requestSource();
    sound_low->setPosition(p);
    sound_low->setLooping(true);
    sound_low->setGain(0.0001);
    /*
    sound_low->play(thegame->getSoundMan()->querySound(
            thegame->getConfig()->query("Tank_engine_sound_low")));
    */

    sound_high = thegame->getSoundMan()->requestSource();
    sound_high->setPosition(p);
    sound_high->setLooping(true);
    sound_high->setGain(0.0001);
    /*
    sound_high->play(thegame->getSoundMan()->querySound(
            thegame->getConfig()->query("Tank_engine_sound_high")));
	*/

    event_sheet = new EventSheet;
    /*
    thegame->getEventRemapper()->map("+primary",
        SigC::bind(
            SigC::slot(*tank_controls, &TankControls::setFire),
            true
            ));
    thegame->getEventRemapper()->map("-primary",
        SigC::bind(
            SigC::slot(*tank_controls, &TankControls::setFire),
            false
            ));
    */
    event_sheet->map("+primary", SigC::slot(armament, &Armament::trigger));
    event_sheet->map("-primary", SigC::slot(armament, &Armament::release));
    event_sheet->map("cycle-primary", SigC::slot(armament, &Armament::nextWeapon));
    event_sheet->map("next-target", SigC::slot(*targeter, &Targeter::selectNextTarget));
    event_sheet->map("previous-target", SigC::slot(*targeter, &Targeter::selectPreviousTarget));
    event_sheet->map("next-hostile-target", SigC::slot(*targeter, &Targeter::selectNextHostileTarget));
    event_sheet->map("previous-hostile-target", SigC::slot(*targeter, &Targeter::selectPreviousHostileTarget));
    event_sheet->map("next-friendly-target", SigC::slot(*targeter, &Targeter::selectNextFriendlyTarget));
    event_sheet->map("previous-friendly-target", SigC::slot(*targeter, &Targeter::selectPreviousFriendlyTarget));
    event_sheet->map("nearest-target", SigC::slot(*targeter, &Targeter::selectNearestTarget));
    event_sheet->map("nearest-hostile-target", SigC::slot(*targeter, &Targeter::selectNearestHostileTarget));
    event_sheet->map("nearest-friendly-target", SigC::slot(*targeter, &Targeter::selectNearestFriendlyTarget));
    event_sheet->map("gunsight-target", SigC::slot(*targeter, &Targeter::selectTargetInGunsight));
}

Tank::~Tank() {
}

void Tank::action() {
    if (state == DEAD) return;
    float delta_t = thegame->getClock()->getStepDelta();

    age+=delta_t;

    Ptr<IActor> target = targeter->getCurrentTarget();

    if (control_mode == AUTOMATIC) {
    	brain->info = "";
	    if (target) {
	        brain->advanced_cannon_control.setMuzzleVelocity(MUZZLE_VELOCITY);
	        brain->advanced_cannon_control.setTarget(target->getLocation());
	        brain->maintain_position.setTarget(target->getLocation(),
	                                           target->getMovementVector());
	    }
	
	    brain->advanced_cannon_control.run(*brain);
	    //brain->maintain_position.run(*brain);
	    brain->move_to_exposed_point.run(*brain);
	    
	    getTargetInfo()->setTargetInfo(brain->info);
	
	    tank_controls->setFire(
	        target
	        && brain->cannon_control.inAimingCone()
	        && (target->getLocation()-getLocation()).lengthSquare()
	            < BULLET_RANGE*BULLET_RANGE);
    } else if (control_mode == MANUAL) {
    	Ptr<EventRemapper> remap = thegame->getEventRemapper();
    	tank_controls->setSteer(remap->getAxis("car_steer"));
    	tank_controls->setThrottle(remap->getAxis("car_throttle"));
    	tank_controls->setBrake(remap->getAxis("car_brake"));
    	tank_controls->setTurretSteer(remap->getAxis("tank_turret_steer"));
    	tank_controls->setCannonSteer(remap->getAxis("tank_cannon_steer"));
    }

    //setTargetInfo(main_idea->getInfo());
    SimpleActor::action();
    armament.action(delta_t);
    
    Vector right, up, front;
    getOrientation(&up, &right, &front);
    Matrix3 M = MatrixFromColumns(right, up, front);
    Quaternion q;
    q.fromMatrix(M);
    skeleton->setBoneTransform("Body", Transform(q, getLocation()));
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
    //sound_high->setPitch(0.5f + 1.5*std::min(1.0f,v/30));
    sound_high->setGain(1000.0000);
    //sound_low->setGain(0.000001 * 0.2f * gain);
    sound_low->setGain(0.00000);
}


#define MAX_MODEL_DISTANCE 3000.0f
#define MAX_POINT_DISTANCE 10000.0f

void Tank::draw() {
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
        renderer->disableAlphaBlending();
        return;
    }
    
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->enableLighting();
    skeleton->draw(*renderer);
    renderer->disableLighting();
}

// Our tank has been hit ...
void Tank::applyDamage(float damage, int domain, Ptr<IProjectile> projectile) {
	if (projectile->getSource()) {
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
    brain->move_to_exposed_point.onEnabled(*brain);
}

bool Tank::hasControlMode(ControlMode) {
  return true;
}
void Tank::setControlMode(ControlMode m) {
    if (control_mode==MANUAL && m!=control_mode)  {
        thegame->getEventRemapper()->removeEventSheet(event_sheet);
    }
    control_mode = m;
    if (m==UNCONTROLLED) {
	    tank_controls->setSteer(0.0f);
	    tank_controls->setThrottle(0.0f);
	    tank_controls->setTurretSteer(0.0f);
	    tank_controls->setCannonSteer(0.0f);
	    tank_controls->setFire(false);
    } else if (m==MANUAL) {
        thegame->getEventRemapper()->addEventSheet(event_sheet);
    }
}

int Tank::getNumViews() {
	return 6;
}

Ptr<IView> Tank::getView(int n) {

    Ptr<FlexibleGunsight> gunsight1 = new FlexibleGunsight(thegame);
    gunsight1->addBasicCrosshairs();
    gunsight1->addDebugInfo(thegame, this);
    gunsight1->addTargeting(this, targeter);
    gunsight1->addArmamentToScreen(thegame, &armament);
    
    Ptr<FlexibleGunsight> gunsight2 = new FlexibleGunsight(thegame);
    gunsight2->addDebugInfo(thegame, this);
    gunsight2->addTargeting(this, targeter);
    gunsight1->addArmamentToScreen(thegame, &armament);
    
    switch(n) {
    case 0:
    	return new RelativeView(
            this,
            Vector(0.0f, 3, 1.5f),
            Vector(1,0,0),
            Vector(0,1,0),
            Vector(0,0,1),
            gunsight2);
    case 1:
        return new FollowingView(
            new SkeletonProvider(skeleton, "CannonCamera",
                "CannonTip", "CannonTipFront",
                "CannonTip", "CannonTipUp"), this, gunsight1);
    case 2:
        return new FollowingView(
            new SkeletonProvider(skeleton, "MGCamera",
                "MG", "MGFront",
                "MG", "MGUp"), this, gunsight1);
    case 3:
    	return new RelativeView(
            this,
            Vector(0,6,18),
            Vector(-1,0,0),
            Vector(0,1,0),
            Vector(0,0,-1),
            gunsight2);
    case 4:
    	return new TurretView(this, tank_engine, gunsight1);
    case 5:
    	return new RelativeView(
            this,
            Vector(0,4,-8),
            Vector(1,0,0),
            Vector(0,1,0),
            Vector(0,0,1),
            gunsight2);
    default:
    	return 0;
    }
}

void Tank::explode() {
    state=DEAD;
    tankFinalExplosion(thegame, this);
}


void Tank::shoot() {
    Vector p_bullet = skeleton->getPoint("CannonTip");
    Vector d_bullet = skeleton->getPoint("CannonTip2") - p_bullet;
    Vector v_bullet = MUZZLE_VELOCITY * d_bullet;

    Ptr<Bullet> bullet = new Bullet(ptr(thegame), this, 2.5f);
    bullet->setTTL(BULLET_TTL);
    bullet->shoot(p_bullet, v_bullet, d_bullet);
    //bullet->setNoCollideParent(this);
    thegame->addActor(bullet);

    Ptr<SoundSource> snd_src = thegame->getSoundMan()->requestSource();
    snd_src->setPosition(p_bullet);
    snd_src->play(thegame->getSoundMan()->querySound(
            thegame->getConfig()->query("Tank_cannon_sound")));
    thegame->getSoundMan()->manage(snd_src);

    /*
    Ptr<SmokeTrail> trail = new SmokeTrail(thegame);
    trail->follow(bullet);
    thegame->addActor(trail);
    thegame->addActor(new Explosion(thegame, p_bullet, 0.5f, 0.0f));
    */
}
