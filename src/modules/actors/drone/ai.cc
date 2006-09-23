#include <cmath>
#include <cstdio>
#include <modules/actors/projectiles/smartmissile.h>
#include <modules/actors/projectiles/bullet.h>
#include <modules/clock/clock.h>
#include <modules/ai/PathManager.h>
#include <interfaces/ITerrain.h>
#include <Faction.h>

#include "ai.h"
#include "drone.h"

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
    ls_warning("Randomized personality: "
               "confidence = %f "
               "obedience = %f "
               "experience = %f "
               "cautiousness = %f\n",
               confidence, obedience, experience, cautiousness);
               
}

PatrolIdea::PatrolIdea(
	Context & ctx,
    const Vector & area,
    float radius)
:	Idea(ctx,"Patrol"), area(area), radius(radius)
{
    //choosePath();
}

Rating PatrolIdea::rate() {
    Rating r;
    r.attack = 0.2;
    r.defense = 0.2;
    r.order = 0.2;
    r.opportunity = 0.0;
    r.necessity = 0.5;
    r.danger = 0.3;
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

    Vector p = ctx.fi->getCurrentLocation();
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
    Vector v = ctx.fi->getCurrentVelocity();
    if (d*v > 0.0f) tan_alpha += 0.0001f*(dist*v);
    d -= dist * tan_alpha;

    float course = atan2(d[0], d[2]);
    if (course < 0) course += 2*PI;

    ctx.ap->setMode( AP_COURSE_MASK | AP_ALTITUDE_MASK | AP_SPEED_MASK );
    ctx.ap->setTargetCourse(course);
    ctx.ap->setTargetAltitude(a[1]+t*(b[1]-a[1]));
    ctx.ap->setTargetSpeed(120.0);
}

void PatrolIdea::postpone() {
    ctx.ap->reset();
}

std::string PatrolIdea::info() {
    char buf[256];
    Vector p = ctx.fi->getCurrentLocation();
    float course = ctx.ap->getTargetCourse();
    course *= 180.0 / PI;
    float current_course = ctx.fi->getCurrentCourse() * 180.0 / PI;
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
        it[-2] = 0.25*(v0+2*v1+v1);
        v0 = v1;
        v1 = v2;
        v2 = *it;
    } while (++it != path.end());
    it[-2] = 0.25*(v0+2*v1+v1);
}
}

void PatrolIdea::choosePath() {
    float alpha = 2.0 * PI * RAND;
    float r = radius * RAND;
    Vector navpoint = area + r*Vector(cos(alpha), 0, sin(alpha));
    Vector dist = navpoint - ctx.fi->getCurrentLocation();
    dist[1] = 0;
    //AI::PathManager pm(500.0f, 500.0f);
    float res = dist.length() / 24.0f;
    res = std::max(500.0f, res);
    AI::PathManager pm(res, res );
    PatrolPathEvaluator pe(ctx.terrain);
    pm.findPath(ctx.actor->getLocation() + 5.0f*ctx.actor->getMovementVector(), navpoint, &pe);
    pm.getPath(&path);
    path.push_front(ctx.fi->getCurrentLocation());
    path.front()[1] = ctx.terrain->getHeightAt(path.front()[0], path.front()[2]) + 400.0f;
    path.back()[1] = ctx.terrain->getHeightAt(path.back()[0], path.back()[2]) + 400.0f;
    for(int i=1; i<path.size()-1; i++) {
        path[i][1] = path[0][1]
            + ((float) i / (path.size()-1)) * (path.back()[1]-path[0][1]);
        if (path[i][1] - ctx.terrain->getHeightAt(path[i][0], path[i][2]) < 200.0f)
            path[i][1] = ctx.terrain->getHeightAt(path[i][0], path[i][2]) + 200.0f;
    }
    for(int i=0; i<1; i++) smoothen_path(path);
}



#define TIME_FOR_TARGET_SELECT 5.0
#define TIME_FOR_MISSILE 15.0
#define TIME_FOR_BULLET 0.1
AttackIdea::AttackIdea(Context & ctx)
: 	Idea(ctx,"Attack")
{
    last_target_select = RAND * TIME_FOR_TARGET_SELECT;
    last_missile = RAND * TIME_FOR_MISSILE;
    last_bullet = RAND * TIME_FOR_BULLET;
    delta_t = 0.0;
    p = ctx.actor->getLocation();
}

#define TARGET_SEEK_DISTANCE 12000.0
#define TARGET_SEEK_DISTANCE_SQUARE = (12000.0*12000.0)
#define MISSILE_SHOOT_ANGLE (45.0 * PI / 180.0)
#define BULLET_SHOOT_ANGLE (1.0 * PI / 180.0)
#define TARGET_SEEK_ANGLE (165.0 * PI / 180.0)
#define BULLET_SPEED 800.0f
#define BULLET_SHOOT_DISTANCE 1000.0f
Rating AttackIdea::rate() {
    Rating rating;
    
    delta_t = ctx.thegame->getTimeDelta()/1000.0;
    
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
        rating.danger += 0.5;
        rating.attack += 1.0;
        rating.opportunity += 0.7;
        
        //ctx.armament.selectWeapon("Vulcan");
        //ctx.secondary.selectWeapon("Sidewinder");
        evaluateSituation();
        
        if (canFireMissile()) {
            rating.attack+=0.3;
            rating.opportunity+=0.4;
        }
        
        if (canFireBullet()) {
            rating.attack+=0.1;
            rating.opportunity+=0.2;
        }
    }
    
    return rating;
}

void AttackIdea::realize() {
    //ctx.primary.release();
    //ctx.secondary.release();
    if (target && target->getState() == IActor::DEAD) {
        target=0;
        return;
    }
    ctx.targeter->setCurrentTarget(target);
    //ctx.primary.selectWeapon("Vulcan");
    //ctx.secondary.selectWeapon("Sidewinder");
    if (target) followTarget();

    if (canFireMissile()) fireMissile();
    else if (canFireBullet()) fireBullet();
}

void AttackIdea::postpone() {
    //ctx.primary.release();
    //ctx.secondary.release();
    ctx.ap->reset();
}

std::string AttackIdea::info() {
    if (target) {
        Vector target_pos = target->getLocation();
        Vector d = target_pos - p;
        Vector v = ctx.actor->getMovementVector();
        float dist = d.length();
        d.normalize();
        v.normalize();
        float angle = acos(d*v) * 180.0 / PI;
        float course = atan2(d[0], d[2]);
        float my_course = ctx.fi->getCurrentCourse() * 180.0 /PI;
        if (course < 0) course += 2.0*PI;
        course *= 180.0 / PI;
        //float pitch = asin( Vector(0,1,0)*d ) * 180.0 / PI;
        float pitch = ctx.ap->getTargetPitch() * 180.0 / PI;

        char buf[256];
        snprintf(buf, 256, "attacking %s %s\n"
                "target position %7.2f %7.2f %7.2f\n"
                "own position %7.2f %7.2f %7.2f\n"
                "own speed: %4.2f target speed: %4.2f\n"
                "course to target %5.2f? current course %5.2f\n"
                "angle %5.2f dist %7.2f\n"
                "pitch %5.2f\n",
                target->getTargetInfo()->getType().getName(),
                target->getTargetInfo()->getTargetName().c_str(),
                target_pos[0], target_pos[1], target_pos[2],
                p[0], p[1], p[2],
                ctx.actor->getMovementVector().length(),
                ctx.ap->getTargetSpeed(),
                course, my_course, angle, dist, pitch);
        
        return buf;
    } else return "attacking";
}

void AttackIdea::evaluateSituation() {
    p = ctx.actor->getLocation();
    v = ctx.actor->getMovementVector();
    front = ctx.actor->getFrontVector();
    
    double delta_t = ctx.thegame->getClock()->getStepDelta();
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
    bool can_fire = target /*&& ctx.secondary.currentWeapon()->canFire()*/ &&
            last_missile > TIME_FOR_MISSILE &&
            target_angle < MISSILE_SHOOT_ANGLE;
    if (!can_fire) ls_message("Can't fire!\n");
    return can_fire;
}

void AttackIdea::fireMissile() {
    if (!target) return;
    //ctx.secondary.trigger();
    last_missile = 0;
}

bool AttackIdea::canFireBullet() {
    return target /*&& ctx.primary.currentWeapon()->canFire()*/ &&
            last_bullet > TIME_FOR_BULLET &&
            target_angle < BULLET_SHOOT_ANGLE &&
            target_dist < BULLET_SHOOT_DISTANCE;
}

void AttackIdea::fireBullet() {
    if (!target) return;
    //ctx.primary.trigger();
}

bool AttackIdea::canSelectTarget() {
    return !target && last_target_select > TIME_FOR_TARGET_SELECT;
}

void AttackIdea::selectTarget() {
    typedef IActorStage::ActorVector List;
    typedef List::const_iterator Iter;
    List list;
    ctx.thegame->queryActorsInSphere(
        list,
        ctx.actor->getLocation(),
        TARGET_SEEK_DISTANCE);
        
    float best_dist = 0;
    Ptr<IActor> best_target;

    Vector v = ctx.actor->getMovementVector();
    v.normalize();

    for (Iter i=list.begin(); i!=list.end(); i++) {
        Ptr<IActor> actor = *i;
        if (actor == ctx.actor) continue;
        if(! actor->getTargetInfo() ) continue;
        if(!actor->getTargetInfo()->isA(TargetInfo::DETECTABLE))
        	continue;
        if(ctx.actor->getFaction()->getAttitudeTowards(actor->getFaction())
            != Faction::HOSTILE) continue;
        Vector d = p - actor->getLocation();
        float dist = d.length();
        float angle = acos(v*d/dist);
        if (angle > TARGET_SEEK_ANGLE) {
            continue;
        }

        if ( best_dist==0 || dist < best_dist ) {
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

    ctx.ap->setMode(AP_PITCH_MASK | AP_COURSE_MASK | AP_SPEED_MASK);

    float pitch = asin( up*d );
    ctx.ap->setTargetPitch(pitch);

    float course = atan2(d[0], d[2]);
    if (course < 0) course += 2.0*PI;
    
    ctx.ap->setTargetCourse(course);
    float speed = target->getMovementVector().length() + (dist - 800.0) * 5.0;
    speed = std::max(100.0f, speed);
    ctx.ap->setTargetSpeed(speed);
}


Rating EvadeTerrainIdea::rate() {
    Rating rating;
    
    float height = ctx.fi->getCurrentHeight();
    
    if (triggered && height > 200) triggered=false;
    
    if (ctx.fi->collisionWarning()) {
        rating.defense += 0.4;
        rating.danger += 0.9;
        rating.necessity += 0.7;
    }
    
    if (height < 150.0) {
        float x = height / 150.0;
        x *= x;
        x = 1.0-x;
        rating.necessity += 3.0*x;
        rating.danger += 3.0*x;
    }
    
    rating.necessity += triggered?1:0;
    
    return rating;
}

void EvadeTerrainIdea::realize() {
	triggered=true;
    if (!ctx.fi->collisionWarning()) {
        ctx.ap->setMode(AP_HEIGHT_MASK);
        ctx.ap->setTargetHeight(400.0);
    } else {
        ctx.ap->setMode(AP_PITCH_MASK | AP_ROLL_MASK);
        ctx.ap->setTargetPitch(70*PI/180);
        ctx.ap->setTargetRoll(0);
    }
    ctx.controls->setThrottle(1);
    ctx.controls->setRudder(0);
}

void EvadeTerrainIdea::postpone() {
    ctx.ap->reset();
}

std::string EvadeTerrainIdea::info() {
	std::string nfo;
    if (ctx.fi->collisionWarning()) return "Pulling up";
    else return "Evading terrain";
}


Rating CRIdea::rate() {
	Rating r;
	r.attack = 1;
	r.defense = 1;
	r.order = 1;
	r.necessity = 1;
	r.opportunity = 1;
	return r;
}

void CRIdea::realize() {
	start();
}

void CRIdea::postpone() {
	CoRoutine::interrupt();
}

std::string CRIdea::info() {
	return nfo;
	ctx.ap->reset();
}

void CRIdea::run() {
	for(;;) {
		nfo = "Stabilizing";
		ctx.ap->setMode(AP_ROLL_MASK|AP_PITCH_MASK|AP_SPEED_MASK);
		ctx.ap->setTargetRoll(0);
		ctx.ap->setTargetPitch(0);
		ctx.ap->setTargetSpeed(200);
		
		do {
			yield();
		} while (
			std::abs(ctx.fi->getCurrentRoll()) > 0.02
			|| std::abs(ctx.fi->getCurrentPitch()) > 0.05
			|| std::abs(ctx.fi->getCurrentSpeed()) < 120);
			
		nfo = "Rolling";
		ctx.ap->reset();
		ctx.controls->setAileron(1);
		do {
			yield();
		} while (std::abs(ctx.fi->getCurrentRoll()) < 1.7);
		do {
			yield();
		} while (std::abs(ctx.fi->getCurrentRoll()) > 0.5);
		
		nfo = "Stablilizing";
		ctx.ap->setMode(AP_ROLL_MASK|AP_PITCH_MASK|AP_SPEED_MASK);
		ctx.ap->setTargetRoll(0);
		ctx.ap->setTargetPitch(0);
		ctx.ap->setTargetSpeed(200);
		
		do {
			yield();
		} while (
			std::abs(ctx.fi->getCurrentRoll()) > 0.02
			|| std::abs(ctx.fi->getCurrentPitch()) > 0.05
			|| std::abs(ctx.fi->getCurrentSpeed()) < 120);
			
		nfo = "Looping";
		ctx.ap->reset();
		
		ctx.controls->setThrottle(1);
		ctx.controls->setAileronAndRudder(0);
		ctx.controls->setElevator(-1);
		
		do {
			yield();
		} while (ctx.actor->getUpVector()[1] > -0.01);
		do {
			yield();
		} while (ctx.actor->getUpVector()[1] < 0.5);
		
		nfo = "Gaining Height";
		ctx.ap->setMode(AP_HEIGHT_MASK|AP_SPEED_MASK);
		ctx.ap->setTargetHeight(1000);
		ctx.ap->setTargetSpeed(200);
		do {
			yield();
		} while (
			std::abs(ctx.fi->getCurrentHeight()) < 995
			|| std::abs(ctx.fi->getCurrentSpeed()) < 120);
		
	}
}


Rating Dogfight::rate() {
	Rating r;
	Ptr<IActor> tgt = ctx.targeter->getCurrentTarget();
	if (!tgt || !targetInRange(tgt)) {
		tgt = selectNearestTargetInRange();
	}
	
	if (tgt) {
		float dist = (tgt->getLocation()-ctx.actor->getLocation()).length();
		float factor = dist/12000;
		factor = 1-factor*factor;
		r.attack = 1*factor;
		r.defense = 0.5*factor;
		r.danger = 0.7*factor;
		r.order = 0.3*factor;
		r.necessity = 1*factor;
		r.opportunity = 1*factor;
	}
	return r;
}


void Dogfight::realize() {
	start();
}

void Dogfight::postpone() {
	CoRoutine::interrupt();
	//ctx.controls->setFirePrimary(false);
    ctx.armament.release(0);
}

std::string Dogfight::info() {
	return nfo;
}

void Dogfight::run() {
	for(;;) {
		nfo="Dogfighting";
		if (ctx.targeter->getCurrentTarget() &&
				!targetInRange(ctx.targeter->getCurrentTarget()))
			ctx.targeter->clearCurrentTarget();
		if (!ctx.targeter->getCurrentTarget()) {
			selectNearestTargetInRange();
		}
		if (!ctx.targeter->getCurrentTarget()) {
			yield();
			continue;
		}
		
        if (positionFavorable()) {
			nfo +="\nposition favorable\n";
			attackTarget();
		} else {
			nfo +="\nposition not favorable\n";
			evade();
		}
		
		yield();
	}
}

bool Dogfight::targetInRange(Ptr<IActor> tgt) {
    return ptr(tgt) && tgt->isAlive() && (tgt->getLocation()-ctx.actor->getLocation()).length() < 5000;
}

Ptr<IActor> Dogfight::selectNearestTargetInRange(float range) {
    ctx.targeter->setMaxRange(range);
    ctx.targeter->selectNearestHostileTarget();
    return ctx.targeter->getCurrentTarget();
}	


bool Dogfight::positionFavorable() {
	Vector target_p = ctx.targeter->getCurrentTarget()->getLocation();
	Vector target_dir = ctx.targeter->getCurrentTarget()->getFrontVector();
	Vector own_p = ctx.actor->getLocation();
	Vector own_dir = ctx.actor->getFrontVector();

    if (own_p[0] != own_p[0]) {
        ls_error("Trouble: own pos %f %f %f\n",own_p[0],own_p[1],own_p[2]);
    }
	
	Vector d = target_p-own_p;
	float dist = d.length();
	d /= dist;
	
	bool in_front_of_target = -d*target_dir > std::cos(30*PI/180);
	bool target_behind = d*own_dir < std::cos(120*PI/180);
	
	return dist>100 || !in_front_of_target && !target_behind;
}

void Dogfight::attackTarget() {
    //ctx.primary.selectWeapon("Vulcan");
    //ctx.secondary.selectWeapon("Sidewinder");

    Ptr<Weapon> missiles = ctx.armament.getWeapon("Sidewinder");
    if (missiles && missiles->getRoundsLeft() > 0 &&
        (ctx.targeter->getCurrentTarget()->getLocation() - ctx.actor->getLocation()).length() > 1000)
    {
        missileAttack(missiles);
    } else {
        Ptr<Weapon> cannon = ctx.armament.getWeapon("Vulcan");
        aimAndShoot(cannon);
    }
}

void Dogfight::aimInDirection(Vector d) {
    float pitch = asin( d[1] );
    ctx.ap->setTargetPitch(pitch);

    float course = atan2(d[0], d[2]);
    if (course < 0) course += 2.0*PI;
    ctx.ap->setTargetCourse(course);
    char buf[512];
    snprintf(buf,512, "%s\n\n"
    	"aiming in direction: %3.2f\n"
    	"with pitch: %3.2f", nfo.c_str(), course*180/PI, pitch);
    nfo = buf;
}

void Dogfight::aimAndShoot(Ptr<Weapon> cannon) {
	char buf[1024];
	nfo += "aiming and shooting";
	ctx.ap->setMode(AP_SPEED_MASK|AP_COURSE_MASK|AP_PITCH_MASK);
	while (positionFavorable() && targetInRange(ctx.targeter->getCurrentTarget())) {
		Vector p = ctx.actor->getLocation();
		Vector v = ctx.actor->getMovementVector();
		Vector target_p = ctx.targeter->getCurrentTarget()->getLocation();
		Vector target_v = ctx.targeter->getCurrentTarget()->getMovementVector();
	    
	    Vector target_dir = target_p-p;
	    float dist = target_dir.length();
	    target_dir /= dist;
	    Vector target_vrel = target_v - target_dir*(target_dir*target_v);
	    float target_vrel2 = target_vrel.lengthSquare();
	    float v2 = v.length() + BULLET_SPEED;
	    v2 *= v2;
	    if (target_vrel2 > v2)
	    	target_vrel2 = v2;
	    Vector desired_dir = target_vrel + target_dir * sqrt(v2 - target_vrel2);
	    desired_dir.normalize();
		
		aimInDirection(desired_dir);
		
	    float speed_diff = (target_v-v)
	    	*ctx.actor->getFrontVector();
	    float speed = target_v.length() + (dist - 500.0) + 15*speed_diff;
	    
	    speed = std::max(100.0f, speed);
	    ctx.ap->setTargetSpeed(speed);
	    
	    float cos_error = desired_dir*ctx.actor->getFrontVector();
	    float tan_error = sqrt(1-cos_error*cos_error)/cos_error;
	    
        if(cos_error > 0.95 && tan_error <= 50/dist && dist < 800) {
            cannon->trigger();
        } else {
            cannon->release();
        }
	    
		snprintf(buf, 1024, "Aiming and shooting\n"
			"p: %5.2f %5.2f %5.2f\n"
			"target: %5.2f %5.2f %5.2f\n"
			"dir to target: %5.2f %5.2f %5.2f\n"
			"dir: %5.2f %5.2f %5.2f\n"
			"tan_error: %5.3f\n"
			"50/dist: %5.3f",
			p[0], p[1], p[2],
			target_p[0], target_p[1], target_p[2],
			desired_dir[0], desired_dir[1], desired_dir[2],
			ctx.actor->getFrontVector()[0],ctx.actor->getFrontVector()[1],ctx.actor->getFrontVector()[2],
			tan_error,
			50/dist);
		nfo += buf;
		
	    yield();
	}
	ctx.ap->reset();
    cannon->release();
}

void Dogfight::evade() {
	Vector p_me = ctx.actor->getLocation();
	Vector p_target = ctx.targeter->getCurrentTarget()->getLocation();
	Vector delta_p = p_target - p_me;
	Vector v_me = ctx.actor->getMovementVector();
	Vector v_target = ctx.targeter->getCurrentTarget()->getMovementVector();
	
	bool faster_than_enemy = 
		v_me.length() > v_target.length();
	bool behind_enemy = delta_p * v_target > 0;
	bool enemy_behind = delta_p * v_me < 0;
	bool high_altitude = ctx.fi->getCurrentHeight() > 1500;
	bool enemy_above = delta_p[1] > 0;
		
	if (behind_enemy) {
		if (enemy_behind) {
			gainSpeedAndStabilize();
			if (enemy_above || !high_altitude) {
				flyImmelmannTurn();
			} else {
				flySplitS();
			}
		} else if (faster_than_enemy) {
			float r = 3*RAND;
			if (r < 2)
				flyScissors();
			else {
				gainSpeedAndStabilize();
				flyLooping();
			}
		} else {
			gainSpeedAndStabilize();
		}
	} else {
		if (enemy_behind) {
			float r = 7*RAND;
			if (r<1) {
				gainSpeedAndStabilize();
				flyLooping();
			} else if (r<2) {
				flyScissors();
			} else if (r<3) {
				gainSpeedAndStabilize();
				flyImmelmann();
			} else if (r<4 || !high_altitude) {
				gainSpeedAndStabilize();
				flyImmelmannTurn();
			} else {
				stabilize();
				flySplitS();
			}
		}
	}
}

void Dogfight::gainSpeedAndStabilize() {
	ctx.controls->setAileronAndRudder(0);
	ctx.controls->setElevator(0);
	ctx.ap->setMode(AP_PITCH_MASK|AP_ROLL_MASK|AP_SPEED_MASK);
	ctx.ap->setTargetRoll(0);
	ctx.ap->setTargetPitch(0);
	ctx.ap->setTargetSpeed(200);
	do {
		char buf[256];
		snprintf(buf, 256,
			"Gaining speed and stabilizing:\n"
			" roll:  %f > 0.02\n"
			" pitch: %f > 0.08\n"
			" speed: %f < 120",
			ctx.fi->getCurrentRoll(),
			ctx.fi->getCurrentPitch(),
			ctx.fi->getCurrentSpeed());
		nfo = buf;
			
		//nfo = "Gaining speed and stabilizing";
		yield();
		} while (
			std::abs(ctx.fi->getCurrentRoll()) > 0.02
			|| std::abs(ctx.fi->getCurrentPitch()) > 0.08
			|| std::abs(ctx.fi->getCurrentSpeed()) < 120);
	ctx.ap->reset();
}

void Dogfight::stabilize() {
	nfo += "\nStabilizing";
	ctx.controls->setAileronAndRudder(0);
	ctx.controls->setElevator(0);
	ctx.ap->setMode(AP_PITCH_MASK|AP_ROLL_MASK);
	ctx.ap->setTargetRoll(0);
	ctx.ap->setTargetPitch(0);
	do {
		yield();
		} while (
			std::abs(ctx.fi->getCurrentRoll()) > 0.02
			|| std::abs(ctx.fi->getCurrentPitch()) > 0.05);
	ctx.ap->reset();
}

void Dogfight::flyLooping() {
	nfo += "\nLooping";
	ctx.controls->setAileronAndRudder(0);
	ctx.controls->setElevator(-1);
	
	do {
		yield();
	} while (ctx.actor->getUpVector()[1] > -0.01);
	do {
		yield();
	} while (ctx.actor->getUpVector()[1] < 0.5);
}

void Dogfight::flyScissors() {
	nfo+="\nScissors";
	char buf[1024];
	
	ctx.ap->setMode(AP_HEIGHT_MASK|AP_COURSE_MASK);
	ctx.controls->setThrottle(1);
	ctx.ap->setTargetHeight(ctx.fi->getCurrentHeight());
	
	Vector d = ctx.actor->getFrontVector();
	d[1] = 0;
	d.normalize();
    float course = atan2(d[0], d[2]);
    float course_left = course - 0.8;
    float course_right = course + 0.8;
    if (course_left < 0) course_left += 2*PI;
    if (course_right >= 2*PI) course_right -= 2*PI;
	
	ctx.ap->setTargetCourse(course_left);
	float error;
	do {
		yield();
		d = ctx.actor->getFrontVector();
		d[1] = 0;
		d.normalize();
	    float course = atan2(d[0], d[2]);
	    error = course_left - course;
	    if (error < -PI) error += 2*PI;
	    if (error > PI) error -= 2*PI;
	} while (error < 0);
	ctx.ap->setTargetCourse(course_right);
	do {
		yield();
		d = ctx.actor->getFrontVector();
		d[1] = 0;
		d.normalize();
	    float course = atan2(d[0], d[2]);
	    error = course_right - course;
	    if (error < -PI) error += 2*PI;
	    if (error > PI) error -= 2*PI;
	} while (error > 0);
    
	ctx.ap->reset();
}


void Dogfight::flyImmelmann() {
	nfo += "\nImmelmann pulling up.";
	ctx.ap->setMode(AP_PITCH_MASK|AP_ROLL_MASK);
	ctx.ap->setTargetRoll(0);
	ctx.ap->setTargetPitch(85*PI/180);
	ctx.controls->setElevator(-1);
	ctx.controls->setThrottle(1);
	do {
		yield();
	} while (ctx.fi->getCurrentPitch() < 80*PI/180);
	
	nfo += "\nImmelmann rotating.";
	ctx.ap->reset();
	float dest_course = 2*PI*RAND;
	float course;
	ctx.controls->setElevator(0);
	ctx.controls->setAileron(1);
	float error;
	do {
		yield();
		Vector up = ctx.actor->getUpVector();
		up[1]=0;
		up.normalize();
		course = atan2(up[2],up[0]);
		error = course-dest_course;
	    if (error < -PI) error += 2*PI;
	    if (error > PI) error -= 2*PI;
	} while (std::abs(error) > 5*PI/180);
	
	nfo += "\nImmelmann breaking out.";
	ctx.controls->setAileron(0);
	ctx.controls->setElevator(-1);
	ctx.controls->setThrottle(0);
	do{
		yield();
	} while (ctx.actor->getUpVector()[1] > -0.5);
	do{
		yield();
	} while (ctx.actor->getUpVector()[1] < 0.5);
	
}

#define PI 3.141593f

void Dogfight::flyImmelmannTurn() {
	nfo = "Immelmann turn pulling up.";
	ctx.ap->reset();
	ctx.controls->setElevator(-1);
	do {
		yield();
	} while (ctx.actor->getUpVector()[1] > sinf(-70*PI/180));
	
	nfo = "Immelmann turn rotating.";
	ctx.ap->setMode(AP_PITCH_MASK|AP_ROLL_MASK);
	ctx.ap->setTargetPitch(0);
	ctx.ap->setTargetRoll(0);
	do {
		yield();
	} while (ctx.actor->getUpVector()[1] < sinf(60*PI/180));
}

void Dogfight::missileAttack(Ptr<Weapon> missiles) {
    if (!missiles) return;
    double time_until_fire_again=0;

	char buf[1024];
	ctx.ap->setMode(AP_COURSE_MASK|AP_PITCH_MASK);
	while (positionFavorable() && targetInRange(ctx.targeter->getCurrentTarget())) {
		Vector p = ctx.actor->getLocation();
		Vector v = ctx.actor->getMovementVector();
		Vector target_p = ctx.targeter->getCurrentTarget()->getLocation();
		Vector target_v = ctx.targeter->getCurrentTarget()->getMovementVector();
	    
	    Vector target_dir = target_p-p;
	    Vector desired_dir = target_dir;
	    desired_dir.normalize();
		
		aimInDirection(desired_dir);
		
        if (missiles->getRoundsLeft() == 0)
            break;

        if (ctx.actor->getFrontVector() * desired_dir > cosf(15.0f*PI/180) &&
            missiles->canFire() &&
            time_until_fire_again <= 0)
        {
            missiles->trigger();
            time_until_fire_again = 4.0;
        }

		snprintf(buf, 1024, "Aiming for missile attack\n"
			"p: %5.2f %5.2f %5.2f\n"
			"target: %5.2f %5.2f %5.2f\n"
			"desired dir: %5.2f %5.2f %5.2f\n"
			"dir: %5.2f %5.2f %5.2f\n"
			"error: %3.3f°\n"
			"dist: %5.3f",
			p[0], p[1], p[2],
			target_p[0], target_p[1], target_p[2],
			desired_dir[0], desired_dir[1], desired_dir[2],
			ctx.actor->getFrontVector()[0],ctx.actor->getFrontVector()[1],ctx.actor->getFrontVector()[2],
			acosf(ctx.actor->getFrontVector() * desired_dir),
			(target_p-p).length());
		nfo = buf;
		
	    yield();
        time_until_fire_again -= ctx.thegame->getClock()->getFrameDelta();

	}
	ctx.ap->reset();
}

void Dogfight::flySplitS() {
	nfo = "Split S rotating.";
	ctx.ap->setMode(AP_PITCH_MASK|AP_ROLL_MASK);
	ctx.ap->setTargetPitch(0);
	ctx.ap->setTargetRoll(PI); // upside down
	do {
		yield();
	} while (ctx.actor->getUpVector()[1] > -0.8);
	
	nfo = "Split S pulling up.";
	ctx.ap->reset();
	ctx.controls->setElevator(-1);
	do {
		yield();
	} while (ctx.actor->getUpVector()[1] < 0.8);
}
