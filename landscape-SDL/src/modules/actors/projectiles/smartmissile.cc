#include "smartmissile.h"
#include <modules/actors/fx/explosion.h>
#include <modules/engines/missileengine.h>

#define BLAST_BEGIN 1.0
#define BLAST_END 30.0
#define BLAST_THRUST 74000.0
#define MIN_EXPLOSION_AGE 1.0f
#define MAX_LIFETIME 40.0
#define PI 3.14159265358979323846264338327f
#define SCAN_ANGLE (140.0 * PI / 180.0)
#define MIN_INTERCEPT_SPEED 250.0
        

SmartMissile::SmartMissile(Ptr<IGame> thegame, Ptr<IActor> target)
: SimpleActor(thegame), target(target), damage(0)
{
    renderer = thegame->getRenderer();
    terrain = thegame->getTerrain();
    setTargetInfo(new TargetInfo(
        "Smart missile", 1.0f, TargetInfo::CLASS_GUIDED_MISSILE));
    engine = new SmartMissileEngine(thegame);
    setEngine(engine);
    if (target) {
        marker = new TargetMarker(thegame);
        Vector target_pos = target->getLocation();
        marker->setPos(target_pos);
        thegame->addActor(marker);
    }

}



#define MAX_ROTATION_SPEED (90.0f*PI/180.0f)
void SmartMissile::action()
{
    double delta_t = thegame->getTimeDelta() / 1000.0;
    age += delta_t;

    if (age > MAX_LIFETIME) {
        ls_message("killed by MAX_LIFETIME.\n");
        explode();
        return;
    }

    if (target && target->getState() == IActor::DEAD) {
        marker->kill();
        target = 0;
    }

    Vector p = getLocation();
    Vector v = getMovementVector();
    Vector d = engine->getDirection();

    // Aiming
    if (target) {
        Vector target_pos = target->getLocation();
        Vector target_speed = target->getMovementVector();

        if (age>MIN_EXPLOSION_AGE
                && target->getTargetInfo()
                && (target_pos - p).length() <
                    target->getTargetInfo()->getTargetSize()) {
            ls_message("killed by collision with target.\n");
            explode();
            return;
        }

        rendezvous.updateSource(p, Vector(0,0,0), Vector(0,0,0));
        rendezvous.updateTarget(delta_t, target_pos, target_speed);
        rendezvous.setVelocity(1000.0f);
        Vector rendezvous_point;
        if (v.length() > MIN_INTERCEPT_SPEED) {
            rendezvous_point = rendezvous.calculate();
        } else {
            rendezvous_point = target_pos;
        }

        Vector target_direction = (target_pos-p).normalize();

        if (target_direction * Vector(v).normalize() > cos(SCAN_ANGLE)) {
            engine->setTarget(rendezvous_point);
            marker->setPos(rendezvous_point);
        } else {
            if (age>MIN_EXPLOSION_AGE) {
                ls_message("killed when target got out of SCAN_ANGLE.\n");
                explode();
                return;
            }
        }
    }

    if(age > BLAST_BEGIN && age < BLAST_END) engine->setThrust(BLAST_THRUST);
    else engine->setThrust(0);

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

void SmartMissile::draw()
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
}

void SmartMissile::shoot(
        const Vector &pos,
        const Vector &vec,
        const Vector &dir)
{
    engine->setLocation(pos);
    engine->setMovementVector(vec);
    engine->setDirection(dir);
    ls_message("shooting missile pos vec dir: \n");
    engine->getLocation().dump();
    engine->getMovementVector().dump();
    engine->getDirection().dump();

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

void SmartMissile::hitTarget(float damage)
{
    if ((this->damage+=damage) > 0.1) explode();
}

#define MAX_HORIZONTAL_SPEED 600.0
#define MAX_VERTICAL_SPEED 600.0
#define RAND ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)
#define RAND_POS ((float) rand() / (float) RAND_MAX)

#define DAMAGE_RADIUS 80.0f
#define MAX_DAMAGE 2.0f
void SmartMissile::explode()
{
    thegame->addActor(new Explosion(thegame, getLocation(), 10.0));
    if(marker) marker->kill();
    state = DEAD;

    if (target) {
        float dist = (target->getLocation() - getLocation()).length();
        dist = std::min(dist, DAMAGE_RADIUS);
        float damage = (1.0 - dist / DAMAGE_RADIUS);
        damage *= MAX_DAMAGE;
        target->applyDamage(damage);
    }
}
    
