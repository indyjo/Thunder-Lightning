#include <string>
#include <modules/clock/clock.h>
#include <modules/actors/fx/smokecolumn.h>
#include <modules/actors/fx/explosion.h>
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

#define PI 3.14159265358979323846

#define RADIUS 6.0f

#define RAND ((float) rand() / (float) RAND_MAX)
#define RAND2 ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)

#define MUZZLE_VELOCITY 600.0f
#define BULLET_RANGE 2000.0f
#define BULLET_TTL (BULLET_RANGE / MUZZLE_VELOCITY)

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
	    *pos = subject->getLocation() + (*orient) * Vector(0,15,-30);
	}
};



Tank::Tank(Ptr<IGame> thegame)
: SimpleActor(thegame),
  renderer(thegame->getRenderer()),
  terrain(thegame->getTerrain()), damage(0),
  age(0), control_mode(UNCONTROLLED)
{
    setTargetInfo(new TargetInfo(
        "Tank", RADIUS, TargetInfo::TANK));

    tank_controls = new TankControls();
    setControlMode(AUTOMATIC);

    tank_engine = new TankEngine(thegame, tank_controls);
    tank_engine->getFireSignal().connect(
        SigC::slot(*this, &Tank::shoot));
    setEngine(tank_engine);

    brain = new TankBrain(thegame, thegame->getClock(),
                          this, tank_controls, tank_engine);
    //brain->ballistic_cannon_control.onEnabled(*brain);
    //brain->maintain_position.onEnabled(*brain);
    brain->move_to_exposed_point.onEnabled(*brain);

    Vector p = Vector(0, 0, 1000);
    Vector v = Vector(0, 0, 0);
    setLocation(p);
    setMovementVector(v);

    std::string model_path = thegame->getConfig()->query("Tank_model_path");
    std::string base_file = thegame->getConfig()->query("Tank_base_file");
    std::string turret_file = thegame->getConfig()->query("Tank_tower_file");
    std::string cannon_file = thegame->getConfig()->query("Tank_cannon_file");

    base = thegame->getModelMan()->query(base_file);
    turret = thegame->getModelMan()->query(turret_file);
    cannon = thegame->getModelMan()->query(cannon_file);

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
    
	
}

void Tank::action() {
    if (state == DEAD) return;
    double delta_t = thegame->getClock()->getStepDelta();

    age+=delta_t;

    //target = thegame->getGunsight()->getCurrentTarget();
    if (target && target->getState() == IActor::DEAD) target = 0;

    if (control_mode == AUTOMATIC) {
	    if (target) {
	        brain->advanced_cannon_control.setMuzzleVelocity(MUZZLE_VELOCITY);
	        brain->advanced_cannon_control.setTarget(target->getLocation());
	        brain->maintain_position.setTarget(target->getLocation(),
	                                           target->getMovementVector());
	    }
	
	    brain->advanced_cannon_control.run(*brain);
	    //brain->maintain_position.run(*brain);
	    brain->move_to_exposed_point.run(*brain);
	
	    tank_controls->setFire(
	        target
	        && brain->cannon_control.inAimingCone()
	        && (target->getLocation()-getLocation()).lengthSquare()
	            < BULLET_RANGE*BULLET_RANGE);
    } else if (control_mode == MANUAL) {
    	EventRemapper *remap = thegame->getEventRemapper();
    	tank_controls->setSteer(remap->getAxis("car_steer"));
    	tank_controls->setThrottle(remap->getAxis("car_throttle"));
    	tank_controls->setBrake(remap->getAxis("car_brake"));
    	tank_controls->setTurretSteer(remap->getAxis("tank_turret_steer"));
    	tank_controls->setCannonSteer(remap->getAxis("tank_cannon_steer"));
    }

    //setTargetInfo(main_idea->getInfo());
    SimpleActor::action();
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

    Matrix Init = Matrix::Hom(MatrixFromColumns<float>(
            Vector(-1, 0, 0),
            Vector( 0, 0,-1),
            Vector( 0, 1, 0)));
    Matrix Translation = TranslateMatrix<4, float>(p);

    Vector right, up, front;
    getOrientation(&up, &right, &front);
    Matrix Rotation    = Matrix::Hom(
        MatrixFromColumns(right, up, front));
    //Matrix Rotation = Matrix::IdentityMatrix();
    //ls_message("right: "); right.dump();
    //ls_message("up: "); up.dump();
    //ls_message("front: "); front.dump();

    Matrix Mnormal = Rotation * Init;
    Matrix Mmodel  = Translation * Mnormal;
    thegame->getRenderer()->setCullMode(JR_CULLMODE_CULL_POSITIVE);
    base->draw(*renderer, Mmodel, Mnormal);

    Vector turret_pivot(0.0f, -0.424f, 2.297f);
    Vector cannon_pivot(0.0f, -0.424f, 2.413f);

    Mmodel  = Mmodel
            * TranslateMatrix<4,float>(turret_pivot)
            * Matrix::Hom(
                RotateZMatrix<float>(-tank_engine->getTurretAngle()))
            * TranslateMatrix<4,float>(-turret_pivot);

    Mnormal = Mnormal * Matrix::Hom(
        RotateZMatrix<float>(-tank_engine->getTurretAngle()));
    turret->draw(*renderer, Mmodel, Mnormal);

    Mmodel  = Mmodel
            * TranslateMatrix<4,float>(cannon_pivot)
            * Matrix::Hom(
                RotateXMatrix(-tank_engine->getCannonAngle()))
            * TranslateMatrix<4,float>(-cannon_pivot);
    Mnormal = Mnormal * Matrix::Hom(
        RotateXMatrix(-tank_engine->getCannonAngle()));
    cannon->draw(*renderer, Mmodel, Mnormal);
}

// Our tank has been hit ...
void Tank::hitTarget(float damage) {
    if (this->damage < 0.7 && this->damage+damage>0.7) {
        SmokeColumn::PuffParams pparams;
        pparams.color = Vector(0.6f, 0.6f, 0.6f);
        Ptr<FollowingSmokeColumn> smoke =
                new FollowingSmokeColumn(thegame, SmokeColumn::Params(), pparams);
        smoke->follow(this);
        thegame->addActor(smoke);
    }
    this->damage += damage*0.1;
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
	return 5;
}

Ptr<IView> Tank::getView(int n) {

    Ptr<FlexibleGunsight> gunsight1 = new FlexibleGunsight(thegame);
    gunsight1->addBasicCrosshairs();
    gunsight1->addDebugInfo(thegame, this);
    
    Ptr<FlexibleGunsight> gunsight2 = new FlexibleGunsight(thegame);
    gunsight2->addDebugInfo(thegame, this);
    
    switch(n) {
    case 0:
    	return new RelativeView(
            this,
            Vector(0.0f, 1.5f, 1.5f),
            Vector(1,0,0),
            Vector(0,1,0),
            Vector(0,0,1),
            gunsight2);
    case 1:
    	return new CannonView(this, tank_engine, gunsight1);
    case 2:
    	return new RelativeView(
            this,
            Vector(0,8,-12),
            Vector(1,0,0),
            Vector(0,1,0),
            Vector(0,0,1),
            gunsight2);
    case 3:
    	return new RelativeView(
            this,
            Vector(0,10,18),
            Vector(-1,0,0),
            Vector(0,1,0),
            Vector(0,0,-1),
            gunsight2);
    case 4:
    	return new TurretView(this, tank_engine, gunsight1);
    default:
    	return 0;
    }
}

#define MAX_EXPLOSION_SIZE 6.0
#define MIN_EXPLOSION_SIZE 1.0
#define MAX_EXPLOSION_DISTANCE 40.0
#define NUM_EXPLOSIONS 15
#define MAX_EXPLOSION_AGE -2.0
void Tank::explode() {
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
}


void Tank::shoot() {
    Matrix Init = Matrix::Hom(MatrixFromColumns<float>(
            Vector(-1, 0, 0),
            Vector( 0, 0,-1),
            Vector( 0, 1, 0)));
    Vector p = engine->getLocation();
    Matrix Translation = TranslateMatrix<4,float>(p);

    Vector right, up, front;
    getOrientation(&up, &right, &front);
    Matrix Rotation    = Matrix::Hom(
        MatrixFromColumns<float>(right, up, front));

    Matrix Mmodel  = Translation * Rotation * Init;

    Vector turret_pivot(0.0f, -0.424f, 2.297f);
    Vector cannon_pivot(0.0f, -0.424f, 2.413f);
    Vector cannon_nozzle1(0.0f, -9.278f, 4.097f);
    Vector cannon_nozzle2(0.0f,-10.278f, 4.097f);

    Mmodel  = Mmodel
            * TranslateMatrix<4,float>(turret_pivot)
            * Matrix::Hom(
                RotateZMatrix<float>(-tank_engine->getTurretAngle()))
            * TranslateMatrix<4,float>(-turret_pivot);

    Mmodel  = Mmodel
            * TranslateMatrix<4,float>(cannon_pivot)
            * Matrix::Hom(
                RotateXMatrix(-tank_engine->getCannonAngle()))
            * TranslateMatrix<4,float>(-cannon_pivot);

    Vector p_bullet = Mmodel * cannon_nozzle1;
    Vector d_bullet = Mmodel * cannon_nozzle2 - p_bullet;
    Vector v_bullet = MUZZLE_VELOCITY * d_bullet;

    Ptr<Bullet> bullet = new Bullet(&*thegame);
    bullet->setTTL(BULLET_TTL);
    bullet->shoot(p_bullet, v_bullet, d_bullet);
    thegame->addActor(bullet);

    Ptr<SoundSource> snd_src = thegame->getSoundMan()->requestSource();
    snd_src->setPosition(p_bullet);
    snd_src->play(thegame->getSoundMan()->querySound(
            thegame->getConfig()->query("Tank_cannon_sound")));

    /*
    Ptr<SmokeTrail> trail = new SmokeTrail(thegame);
    trail->follow(bullet);
    thegame->addActor(trail);
    thegame->addActor(new Explosion(thegame, p_bullet, 0.5f, 0.0f));
    */
}
