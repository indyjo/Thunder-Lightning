#include <cmath>
#include <cstdio>
#include "ai.h"
#include <modules/actors/projectiles/smartmissile.h>
#include <modules/actors/projectiles/bullet.h>
#include <modules/clock/clock.h>
#include <modules/ai/PathManager.h>
#include <Faction.h>


float Personality::evaluate(const Rating & r) {
    float x = 0.0;
    float factor;

    // evaluate attack rating

    factor  = .6 * confidence;
    factor += .2 * obedience;
    factor += .2 * experience;
    factor += .0 * cautiousness;
    x += factor * r.attack;

    // evaluate defense rating

    factor  = .0 * confidence;
    factor += .3 * obedience;
    factor += .3 * experience;
    factor += .4 * cautiousness;
    x += factor * r.defense;

    // evaluate order rating

    factor  = .2 * confidence;
    factor += .5 * obedience;
    factor += .1 * experience;
    factor += .2 * cautiousness;
    x += factor * r.order;

    // evaluate opportunity rating

    factor  = .7 * confidence;
    factor += .0 * obedience;
    factor += .3 * experience;
    factor += .2 * cautiousness;
    x += factor * r.opportunity;

    // evaluate necessity rating

    factor  = .2 * confidence;
    factor += .2 * obedience;
    factor += .3 * experience;
    factor += .3 * cautiousness;
    x += factor * r.necessity;

    // evaluate danger rating

    factor  = .0 * confidence;
    factor += .2 * obedience;
    factor += - 0.2 * experience;
    factor += - 0.4 * cautiousness;
    x += factor * r.danger;

    // return evaluated rating
    return x;
}


#define RAND ((float) rand() / (float) RAND_MAX)
void Personality::randomize() {
    float x1 = RAND, x2 = 1.0 - x1;
    float x11 = x1 * RAND, x12 = x1 - x11;
    float x21 = x2 * RAND, x22 = x2 - x21;
    
    confidence = x11;
    obedience = x12;
    experience = x21;
    cautiousness = x22;
}

PatrolIdea::PatrolIdea(
        AutoPilot * ap,
        FlightInfo * fi,
        const Vector & area,
        float radius,
        Ptr<ITerrain> terrain)
: ap(ap), fi(fi), area(area), radius(radius), terrain(terrain)
{
    choosePath();
}

Rating PatrolIdea::rate() {
    Rating r;
    r.attack = 0.1;
    r.defense = 0.3;
    r.order = 0.0;
    r.opportunity = 0.0;
    r.necessity = 0.3;
    r.danger = 0.4;
    return r;
}

namespace {
    float segment_pos(const Vector & a, const Vector & b, const Vector & p) {
        Vector d = b-a;
        return (p-a)*d / d.lengthSquare();
    }
    bool in_segment(const Vector & a, const Vector & b, const Vector & p) {
        float pos = segment_pos(a,b,p);
        return pos >= 0.0f && pos <= 1.0f;
    }
    bool beyond_segment(const Vector & a, const Vector & b, const Vector & p) {
        float pos = segment_pos(a,b,p);
        return pos > 1.0f;
    }
    Vector segment_dist(const Vector & a, const Vector & b, const Vector & p) {
        Vector d = b-a;
        d.normalize();
        float pos = (p-a)*d;
        return (p-a) - pos*d;
    }
}

#define PI 3.14159265358979323846
#define NAV_DIST 100.0
// void PatrolIdea::realize() {
//     Vector p = fi->getCurrentLocation();
//     //ls_message("p: "); p.dump();
//     //ls_message("nav: "); navpoint.dump();
//     Vector navpoint = path.front();
//     if ((navpoint - Vector(p[0], navpoint[1], p[2])).length() < NAV_DIST) {
//         path.pop_front();
//         if (path.empty()) choosePath();
//         navpoint = path.front();
//     }
//     float course = atan2(navpoint[0]-p[0], navpoint[2]-p[2]);
//     if (course < 0) course += 2*PI;
//
//     ap->setMode( AP_COURSE_MASK | AP_HEIGHT_MASK | AP_SPEED_MASK );
//     ap->setTargetCourse(course);
//     ap->setTargetHeight(400.0);
//     ap->setTargetSpeed(400.0);
// }

void PatrolIdea::realize() {
    Vector p = fi->getCurrentLocation();
    p[1] = 0.0f;
    //ls_message("p: "); p.dump();
    //ls_message("nav: "); navpoint.dump();
    while (path.size() >= 2 && beyond_segment(path[0], path[1], p)) path.pop_front();
    while (path.size() >= 3 && in_segment(path[1], path[2], p)) path.pop_front();
    if (path.size()<=1) {
        path.clear();
        choosePath();
        return;
    }

    Vector a,b; // The two active segment vertices
    if (path.size()>=3) {
        float t = segment_pos(path[0], path[1], p);
        a = (1.0f-t) * path[0] + t*path[1];
        b = (1.0f-t) * path[1] + t*path[2];
    } else {
        a = path[0];
        b = path[1];
    }

    Vector dist = segment_dist(a, b, p);
    float t = segment_pos(a,b,p);

    float tan_alpha = 0.001f * dist.length();
    dist.normalize();

    Vector d = (b-a).normalize();
    Vector v = fi->getCurrentVelocity();
    if (d*v > 0.0f) tan_alpha += 0.0001f*(dist*v);
    d -= dist * tan_alpha;

    float course = atan2(d[0], d[2]);
    if (course < 0) course += 2*PI;

    ap->setMode( AP_COURSE_MASK | AP_ALTITUDE_MASK | AP_SPEED_MASK );
    ap->setTargetCourse(course);
    ap->setTargetAltitude(a[1]+t*(b[1]-a[1]));
    ap->setTargetSpeed(400.0);
}

void PatrolIdea::postpone() {
    ap->reset();
}

std::string PatrolIdea::info() {
    char buf[256];
    Vector p = fi->getCurrentLocation();
    float course = ap->getTargetCourse();
    course *= 180.0 / PI;
    float current_course = fi->getCurrentCourse() * 180.0 / PI;
    snprintf(buf, 256, "Patroling.\n"
            "position: %+5.0f %+5.0f %+5.0f\n"
            "course %3.0f current course %3.0f",
            p[0], p[1], p[2], course, current_course);
    char buf2[256];
    char *p1= buf2, *p2= buf;
    for(int i=0; i<std::min((int)path.size(),4); i++) {
        snprintf(p1, 256, "%s\n  wp %d: %+5.0f %+5.0f %+5.0f (%5.0f)",
                p2, i, path[i][0], path[i][1], path[i][2],
                (p-Vector(path[i][0], p[1], path[i][2])).length());
        std::swap(p1,p2);
    }
    return p2;
}

class PatrolPathEvaluator : public AI::PathEvaluator {
    Ptr<ITerrain> terrain;
public:
    PatrolPathEvaluator(Ptr<ITerrain> t) : terrain(t) { }
    virtual float cost(const Vector & a, const Vector & b) {
        Vector d = b-a;
        float h_a = terrain->getHeightAt(a[0],a[2]);
        float h_b = terrain->getHeightAt(b[0],b[2]);
        h_a = std::max(0.0f, h_a - 300.0f);
        h_b = std::max(0.0f, h_b - 300.0f);
        d[1] = h_b - h_a;
        d[1] *= 80.0f;
        return d.length();
    }
};

class PatrolPathEvaluator2 : public AI::PathEvaluator {
    Ptr<ITerrain> terrain;
    FlightInfo *fi;
public:
    PatrolPathEvaluator2(Ptr<ITerrain> t, FlightInfo *fi)
    : terrain(t), fi(fi) { }
    virtual float cost(const Vector & a, const Vector & b) {
        float h = 500.0f;
        Vector v1(a[0],h,a[2]);
        Vector v2(b[0],h,b[2]);
        return terrain->lineCollides(v1, v2, &v2)?-1.0f:(v2-v1).length();
    }
};


namespace {
void smoothen_path(std::deque<Vector> & path) {
    if (path.size() < 4) return;
    Vector v0,v1,v2;
    std::deque<Vector>::iterator it = path.begin();
    v0 = *it++;
    v1 = *it++;
    v2 = *it++;
    do {
        it[-2] = 0.5f*(v0+v2);
        v0 = v1;
        v1 = v2;
        v2 = *it;
    } while (it++ != path.end());
}
}

void PatrolIdea::choosePath() {
    float alpha = 2.0 * PI * RAND;
    float r = radius * RAND;
    Vector navpoint = area + r*Vector(cos(alpha), 0, sin(alpha));
    Vector dist = navpoint - fi->getCurrentLocation();
    dist[1] = 0;
    //AI::PathManager pm(500.0f, 500.0f);
    float res = dist.length() / 24.0f;
    res = std::max(500.0f, res);
    AI::PathManager pm(res, res );
    PatrolPathEvaluator pe(terrain);
    pm.findPath(fi->getCurrentLocation() + 5.0f*fi->getCurrentVelocity(), navpoint, &pe);
    pm.getPath(&path);
    path.push_front(fi->getCurrentLocation());
    path.front()[1] = terrain->getHeightAt(path.front()[0], path.front()[2]) + 400.0f;
    path.back()[1] = terrain->getHeightAt(path.back()[0], path.back()[2]) + 400.0f;
    for(int i=1; i<path.size()-1; i++) {
        path[i][1] = path[0][1]
            + ((float) i / (path.size()-1)) * (path.back()[1]-path[0][1]);
        if (path[i][1] - terrain->getHeightAt(path[i][0], path[i][2]) < 200.0f)
            path[i][1] = terrain->getHeightAt(path[i][0], path[i][2]) + 200.0f;
    }
    for(int i=0; i<1; i++) smoothen_path(path);
}



#define TIME_FOR_TARGET_SELECT 5.0
#define TIME_FOR_MISSILE 15.0
#define TIME_FOR_BULLET 0.1
AttackIdea::AttackIdea(Ptr<IGame> thegame, Ptr<IActor> source,
        AutoPilot *ap, FlightInfo *fi, Ptr<DroneControls> controls)
: thegame(thegame), source(source), ap(ap), fi(fi), controls(controls)
{
    last_target_select = RAND * TIME_FOR_TARGET_SELECT;
    last_missile = RAND * TIME_FOR_MISSILE;
    last_bullet = RAND * TIME_FOR_BULLET;
    delta_t = 0.0;
    p = source->getLocation();
}

#define TARGET_SEEK_DISTANCE 12000.0
#define TARGET_SEEK_DISTANCE_SQUARE = (12000.0*12000.0)
#define MISSILE_SHOOT_ANGLE (45.0 * PI / 180.0)
#define BULLET_SHOOT_ANGLE (1.0 * PI / 180.0)
#define TARGET_SEEK_ANGLE (150.0 * PI / 180.0)
#define BULLET_SPEED 800.0f
Rating AttackIdea::rate() {
    Rating rating;
    
    delta_t = thegame->getTimeDelta()/1000.0;
    
    last_target_select += delta_t;
    last_missile += delta_t;
    last_bullet += delta_t;
    
    if (!target && canSelectTarget()) selectTarget();
    
    rating.attack=0;
    rating.defense=0;
    rating.order=0;
    rating.opportunity=0;
    rating.necessity=0;
    rating.danger=0;
    
    if (target) {
        rating.danger += 0.8;
        rating.attack += 1.0;
        rating.opportunity += 0.7;
        
        evaluateSituation();
        
        /*
        if (canFireMissile()) {
            rating.attack+=0.3;
            rating.opportunity+=0.4;
        }
        
        if (canFireBullet()) {
            rating.attack+=0.1;
            rating.opportunity+=0.2;
        }
        */
    }
    
    return rating;
}

void AttackIdea::realize() {
    controls->setFirePrimary(false);
    if (target && target->getState() == IActor::DEAD) target=0;
    if (target) followTarget();

    if (canFireMissile()) fireMissile();
    else if (canFireBullet()) fireBullet();
}

void AttackIdea::postpone() {
    controls->setFirePrimary(false);
    ap->reset();
}

std::string AttackIdea::info() {
    if (target) {
        Vector target_pos = target->getLocation();
        Vector d = target_pos - p;
        Vector v = source->getMovementVector();
        float dist = d.length();
        d.normalize();
        v.normalize();
        float angle = acos(d*v) * 180.0 / PI;
        float course = atan2(d[0], d[2]);
        float my_course = fi->getCurrentCourse() * 180.0 /PI;
        if (course < 0) course += 2.0*PI;
        course *= 180.0 / PI;
        //float pitch = asin( Vector(0,1,0)*d ) * 180.0 / PI;
        float pitch = ap->getTargetPitch() * 180.0 / PI;

        char buf[256];
        snprintf(buf, 256, "attacking %s %s\n"
                "target position %7.2f %7.2f %7.2f\n"
                "own position %7.2f %7.2f %7.2f\n"
                "own speed: %4.2f target speed: %4.2f\n"
                "course to target %5.2f° current course %5.2f\n"
                "angle %5.2f dist %7.2f\n"
                "pitch %5.2f\n",
                target->getTargetInfo()->getTargetClass().name.c_str(),
                target->getTargetInfo()->getTargetName().c_str(),
                target_pos[0], target_pos[1], target_pos[2],
                p[0], p[1], p[2],
                source->getMovementVector().length(), ap->getTargetSpeed(),
                course, my_course, angle, dist, pitch);
        
        return buf;
    } else return "attacking";
}

void AttackIdea::evaluateSituation() {
    p = source->getLocation();
    v = source->getMovementVector();
    front = source->getFrontVector();
    
    double delta_t = thegame->getClock()->getStepDelta();
    rendezvous.updateTarget(
            delta_t, target->getLocation(), target->getMovementVector());
    rendezvous.updateSource(p, v + BULLET_SPEED*front, Vector(0,0,0));
    target_rendezvous = rendezvous.calculate();

    Vector d = target_rendezvous - p;
    
    target_dist = d.length();
    d.normalize();
    target_angle = acos(d*front);
    if (target_dist > TARGET_SEEK_DISTANCE) {
        target = 0;
        //ls_message("lost the target!\n");
    }
}


bool AttackIdea::canFireMissile() {
    return target &&
            last_missile > TIME_FOR_MISSILE &&
            target_angle < MISSILE_SHOOT_ANGLE;
}

void AttackIdea::fireMissile() {
    if (!target) return;
    if (!source) return;
    Ptr<SmartMissile> missile = new SmartMissile(&*thegame, target);
    missile->shoot(
            source->getLocation(),
            source->getMovementVector(),
            Vector(source->getMovementVector()).normalize());
    thegame->addActor(missile);

    last_missile = 0;
}

bool AttackIdea::canFireBullet() {
    return target &&
            last_bullet > TIME_FOR_BULLET &&
            target_angle < BULLET_SHOOT_ANGLE;
}

void AttackIdea::fireBullet() {
    if (!target) return;
    if (!source) return;
    controls->setFirePrimary(true);
    /*
    Ptr<Bullet> bullet = new Bullet(&*thegame);
    Vector v=source->getMovementVector();
    Vector d=source->getFrontVector();
    d.normalize();
    bullet->shoot(p, v + BULLET_SPEED*d, d);
    thegame->addActor(bullet);

    last_bullet = 0;
    */
}

bool AttackIdea::canSelectTarget() {
    return !target && last_target_select > TIME_FOR_TARGET_SELECT;
}

void AttackIdea::selectTarget() {
    typedef const IActorStage::ActorList List;
    typedef List::const_iterator Iter;
    List & list = thegame->getActorList();

    float best_dist = 0;
    Ptr<IActor> best_target;

    Vector v = source->getMovementVector();
    v.normalize();

    for (Iter i=list.begin(); i!=list.end(); i++) {
        Ptr<IActor> actor = *i;
        if(! actor->getTargetInfo() ) continue;
        const TargetInfo::TargetClass & tclass =
            actor->getTargetInfo()->getTargetClass();
        if(!tclass.is_radar_detectable) continue;
        if(source->getFaction()->getAttitudeTowards(actor->getFaction())
            != Faction::HOSTILE) continue;
        Vector p_target = actor->getLocation();
        Vector d = p - p_target;
        float dist = d.length();
        if (dist > TARGET_SEEK_DISTANCE) continue;
        d.normalize();
        float angle = acos(d*v);
        if (angle > TARGET_SEEK_ANGLE) {
            continue;
        }

        if ( actor!=source && ((best_dist==0) || (dist < best_dist))) {
            best_dist = dist;
            best_target = *i;
        }
    }

    target = best_target;
}

void AttackIdea::followTarget() {
    Vector up(0,1,0);
    Vector d = target_rendezvous - p;
    float dist = d.length();
    d.normalize();

    ap->setMode(AP_PITCH_MASK | AP_COURSE_MASK | AP_SPEED_MASK);

    float pitch = asin( up*d );
    ap->setTargetPitch(pitch);

    float course = atan2(d[0], d[2]);
    if (course < 0) course += 2.0*PI;
    
    ap->setTargetCourse(course);
    float speed = target->getMovementVector().length() + (dist - 800.0) * 5.0;
    speed = std::max(100.0f, speed);
    ap->setTargetSpeed(speed);
}


EvadeTerrainIdea::EvadeTerrainIdea(AutoPilot * ap, FlightInfo * fi)
: ap(ap), fi(fi)
{
}

Rating EvadeTerrainIdea::rate() {
    Rating rating;
    
    if (fi->collisionWarning()) {
        rating.defense += 0.4;
        rating.danger += 0.9;
        rating.necessity += 0.7;
    }
    
    float height;
    if ((height = fi->getCurrentHeight()) < 150.0) {
        float x = height / 150.0;
        x *= x;
        x = 1.0-x;
        rating.necessity += 3.0*x;
        rating.danger += 3.0*x;
    }
    
    return rating;
}

void EvadeTerrainIdea::realize() {
    if (!fi->collisionWarning()) {
        ap->setMode(AP_HEIGHT_MASK);
        ap->setTargetHeight(300.0);
    } else {
        ap->setMode(AP_PITCH_MASK | AP_ROLL_MASK);
        ap->setTargetPitch(20.0);
        ap->setTargetRoll(0);
    }
}

void EvadeTerrainIdea::postpone() {
    ap->reset();
}

std::string EvadeTerrainIdea::info() {
    if (fi->collisionWarning()) return "Pulling up";
    else return "Evading terrain";
}
