#include "smartmissile2.h"
#include <modules/actors/fx/explosion.h>
#include <modules/clock/clock.h>
#include <modules/engines/missileengine.h>
#include <interfaces/ICamera.h>
#include <interfaces/ITerrain.h>
#include <sound.h>

#define BLAST_BEGIN 1.0
#define BLAST_END 2.5
#define BLAST_THRUST 74000.0
#define MIN_EXPLOSION_AGE 1.5f
#define MAX_LIFETIME 30.0f
#define PI 3.14159265358979323846264338327f
#define SCAN_ANGLE (70.0 * PI / 180.0)
#define MIN_INTERCEPT_SPEED 250.0
#define MAX_INTERCEPT_SPEED 500.0


/**
A missile similiar to the AIM-9 Sidewinder missile.
Data taken from:
http://www.chinfo.navy.mil/navpalib/factfile/missiles/wep-side.html
*/


SmartMissile2::SmartMissile2(Ptr<IGame> thegame, Ptr<IActor> target)
:   SimpleActor(thegame), target(target), damage(0), age(0)
{
    renderer = thegame->getRenderer();
    terrain = thegame->getTerrain();
    setTargetInfo(new TargetInfo(
        "Smart missile", 1.0f, TargetInfo::CLASS_GUIDED_MISSILE));

    float front_area = PI * 0.13f*0.13f;
    float side_area = 3.0f * 0.63f*0.63f;
    float m = 85.5f;  // wheight in kg
    float l = 2.89f;  // length in m
    float r = 0.065f; // radius in m
    float Iz = 0.5f*m*r*r;
    float Ix = m*(l*l/12 + r*r/4);

    engine = new MissileEngine2(thegame, front_area, side_area);
    engine->construct(m, Ix, Ix, Iz);
    //engine->construct(m, Ix, Ix, Ix);
    setEngine(engine);

    d_error_old = omega_error_old = Vector(0,0,0);

    // Prepare collidable
    setBoundingGeometry(new Collide::BoundingGeometry(1,1));
    getBoundingGeometry()->setBoundingRadius(0.0f * l);
    setRigidBody(engine);
    setActor(this);

    thegame->getCollisionMan()->add(this);
}


void SmartMissile2::action()
{
    char buf[256];

    float delta_t = thegame->getClock()->getStepDelta();
    age += delta_t;

    if (age > MAX_LIFETIME) {
        ls_message("killed by MAX_LIFETIME.\n");
        explode();
        return;
    }

    if (target && target->getState() == IActor::DEAD)
        target = 0;

    Vector p = getLocation();
    Vector v = getMovementVector();
    Vector d = Vector(v).normalize();

    // Aiming
    if (target) {
        Vector target_pos = target->getLocation();
        Vector target_speed = target->getMovementVector();

        rendezvous.updateSource(p, Vector(0,0,0), Vector(0,0,0));
        rendezvous.updateTarget(delta_t, target_pos, target_speed);
        rendezvous.setVelocity(v.length());
        Vector rendezvous_point;
        Vector dv = v - target_speed;
        float rel_speed = dv.length();
        if (rel_speed > MAX_INTERCEPT_SPEED) {
            rendezvous_point = rendezvous.calculate();
        } else if (rel_speed < MIN_INTERCEPT_SPEED){
            rendezvous_point = target_pos;
        } else {
            float u = (rel_speed-MIN_INTERCEPT_SPEED);
            u /= MAX_INTERCEPT_SPEED - MIN_INTERCEPT_SPEED;
            rendezvous_point = u*rendezvous.calculate() + (1-u)*target_pos;
        }


        Vector target_direction = (rendezvous_point-p).normalize();

        //d = (0.6*getFrontVector() + 0.4*d).normalize();
        if ((target_pos-p).normalize() * getFrontVector() > cos(SCAN_ANGLE)) {
            const static float Ka = 20, Kb = 3, Kc = 4, Kd = 0.005;

            float cos_angle = d * target_direction;
            cos_angle = std::max(-1.0f, std::min(1.0f, cos_angle));
            float angle = acos(cos_angle);
            Vector d_error = (d % target_direction).normalize();
            //ls_message("d_error = %f %f %f\n",
            //    d_error[0],d_error[1],d_error[2]);
            d_error *= angle;
            Vector d_error_dt = (d_error-d_error_old) / delta_t;
            d_error_old = d_error;
            if (age == delta_t) d_error_dt = Vector(0,0,0);

            Vector omega_dest = Ka * d_error + Kb * d_error_dt;
            //ls_message("d_error = %f %f %f\n",
            //    d_error[0],d_error[1],d_error[2]);
            //ls_message("d_error_dt = %f %f %f\n",
            //    d_error_dt[0],d_error_dt[1],d_error_dt[2]);
            //ls_message("--> omega_dest = %f %f %f\n",
            //    omega_dest[0],omega_dest[1],omega_dest[2]);

            Vector omega_error = omega_dest - engine->getAngularVelocity();
            Vector omega_error_dt = (omega_error - omega_error_old) / delta_t;
            omega_error_old = omega_error;
            if (age == delta_t) omega_error_dt = Vector(0,0,0);

            Vector omega_dt = Kc * omega_error + Kd * omega_error_dt;
            //ls_message("omega_error = %f %f %f\n",
            //    omega_error[0],omega_error[1],omega_error[2]);
            //ls_message("omega_error_dt = %f %f %f\n",
            //    omega_error_dt[0],omega_error_dt[1],omega_error_dt[2]);

            //ls_message("Apply torque: %f %f %f\n",
            //    omega_dt[0],omega_dt[1],omega_dt[2]);
            engine->applyTorque(omega_dt);
            //engine->applyAngularAcceleration(0.5f * (Vector(0,1,0)-engine->getAngularVelocity()));

        } else {
            if (age>MIN_EXPLOSION_AGE) {
                ls_message("killed when target got out of SCAN_ANGLE.\n");
                explode();
                return;
            }
        }
    }

    if(age > BLAST_BEGIN && age < BLAST_END)
        engine->applyForce(getFrontVector()*BLAST_THRUST);

    Vector p_old = p;
    SimpleActor::action();
    p = getLocation();

    if (terrain->lineCollides(p_old, p, &p)) {
        //SmokeColumn::PuffParams pparams;
        //pparams.color = Vector(35, 30, 25);
        //thegame->addActor(
        //        new SmokeColumn(thegame, p, SmokeColumn::Params(), pparams));
        ls_message("killed by collision with ground.\n");
        ls_message("location: "); getLocation().dump();
        explode();
    }
}

void SmartMissile2::draw()
{
    static const float points[][3] = {
        {0,0,2.0},
        { .1,  0, 0},
        {  0,-.1, 0},
        {-.1,  0, 0},
        {  0, .1, 0},
        { .5,  0, 0},
        {  0,-.5, 0},
        {-.5,  0, 0},
        {  0, .5, 0}
    };

    static const int indices[][3] = {
        {1,0,2},
        {2,0,3},
        {3,0,4},
        {4,0,1},
        {1,2,3},
        {1,3,4},
        {5,0,1},
        {6,0,2},
        {7,0,3},
        {8,0,4}
    };

    static float colors[][3] = {
        {0,0,0},
        {.5,.5,.5},
        {.8,.8,.8},
        {1,.8,.6}
    };

    static const int col_indices[] = {
        1,
        0,
        1,
        0,
        3,
        3,
        2,
        1,
        2,
        1
    };


    static const int n_indices = sizeof(indices) / sizeof(int) / 3;

    Vector right, up, front;
    getOrientation(&up, &right, &front);
    Vector p = getLocation();

    renderer->enableSmoothShading();
    renderer->disableAlphaBlending();
    renderer->disableTexturing();
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->begin(JR_DRAWMODE_TRIANGLES);
    for(int i=0; i<n_indices; i++) {
        float col[3];
        for(int j=0; j<3; j++) col[j] = colors[col_indices[i]][j];
        renderer->setColor(Vector(col));
        for(int j=0; j<3; j++) {
            renderer->vertex(
                    right * points[indices[i][j]][0] +
                    up    * points[indices[i][j]][1] +
                    front * points[indices[i][j]][2] +
                    p);
        }
    }
    renderer->end();


    p -= 1.4f * front;
    Vector vs = engine->getVelocityAt(p);
    renderer->begin(JR_DRAWMODE_CONNECTED_LINES);
    renderer->setAlpha(1);
    renderer->setColor(Vector(1,0,0));
    *renderer << p << p+vs;
    renderer->end();

    vs = vs - front * (vs*front);
    renderer->begin(JR_DRAWMODE_CONNECTED_LINES);
    renderer->setAlpha(1);
    renderer->setColor(Vector(1,1,0));
    *renderer << p << p+vs;
    renderer->end();

    //applyForceAt(-0.001f * vs * vs.length(), p);

    if (!target) return;
    Vector d = (getMovementVector()).normalize();
    Vector target_direction = (target->getLocation()-getLocation()).normalize();
    Vector d_error = acos(d * target_direction) *
        (d % target_direction).normalize();
    renderer->begin(JR_DRAWMODE_CONNECTED_LINES);
    renderer->setAlpha(1);
    renderer->setColor(Vector(0,0,1));
    *renderer << getLocation() << (getLocation()+100*d_error);
    renderer->end();
}

void SmartMissile2::shoot(
        const Vector &pos,
        const Vector &vec,
        const Vector &dir)
{
    engine->setLocation(pos);
    engine->setMovementVector(vec);
    Vector front = dir;
    Vector right = (Vector(0,1,0) % front).normalize();
    Vector up = (front % right).normalize();
    engine->setOrientation(up, right, front);

    Ptr<SmokeTrail> smoketrail =
            new SmokeTrail(thegame);
    smoketrail->follow(this);
    thegame->addActor(smoketrail);
    age = 0;

    Ptr<SoundMan> soundman = thegame->getSoundMan();
    Ptr<SoundSource> soundsource = soundman->requestSource();
    if (soundsource) {
        soundsource->setPosition(pos);
        soundsource->play( soundman->querySound("missile-shoot-1.wav") );
    }

    if (target) {
        rendezvous.updateSource(pos, vec, Vector(0,0,0));
        rendezvous.updateTarget(
                target->getLocation(),
                target->getMovementVector(),
                Vector(0,0,0));
    }
}

void SmartMissile2::hitTarget(float damage)
{
    if ((this->damage+=damage) > 0.1) explode();
}

#define RAND ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)
#define RAND_POS ((float) rand() / (float) RAND_MAX)

#define DAMAGE_RADIUS 50.0f
#define MAX_DAMAGE 1.5f
void SmartMissile2::explode()
{
    thegame->addActor(new Explosion(thegame, getLocation(), 10.0));
    state = DEAD;
    thegame->getCollisionMan()->remove(this);

    if (target) {
        float dist = (target->getLocation() - getLocation()).length();
        ls_message("target position: ");
        target->getLocation().dump();
        ls_message("p: ");
        getLocation().dump();
        ls_message("dist = %f\n");
        dist = std::min(dist, DAMAGE_RADIUS);
        dist /= DAMAGE_RADIUS;
        dist *= dist;
        ls_message("dist = %f\n");
        float damage = (1.0 - dist);
        damage *= MAX_DAMAGE;
        target->applyDamage(damage);
    }
}


void SmartMissile2::integrate(float delta_t, Transform * transforms) {
    engine->integrate(delta_t, transforms);
}

void SmartMissile2::update(float delta_t, const Transform * new_transforms) {
    engine->update(delta_t, new_transforms);
}

void SmartMissile2::collide(const Collide::Contact & c) {
    Ptr<Collidable> partner;
    if (c.collidables[0] == this)
        partner = c.collidables[1];
    else
        partner = c.collidables[0];
    Ptr<IActor> a = partner->getActor();
    if (age <= MIN_EXPLOSION_AGE) return;
    if (a) a->applyDamage(MAX_DAMAGE);
    explode();
}
