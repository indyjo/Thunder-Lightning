#ifndef AI_H
#define AI_H

#include <deque>
#include <landscape.h>
#include <coroutine.h>
#include <modules/engines/controls.h>
#include <modules/flight/flightinfo.h>
#include <modules/flight/autopilot.h>
#include <modules/math/Rendezvous.h>
#include <modules/weaponsys/Targeter.h>

struct Rating {
    Rating() : attack(0), defense(0),
            order(0), opportunity(0),
            necessity(0),
            danger(0) { }
    float attack;
    float defense;
    float order;
    float opportunity;
    float necessity;
    float danger;
};


struct Context : public Object {
	FlightInfo *fi;
	AutoPilot *ap;
	Ptr<DroneControls> controls;
    Ptr<IGame> thegame;
	Ptr<ITerrain> terrain;
    Ptr<IActor> actor;
    Ptr<Targeter> targeter;
    MTasker<> & mtasker;
    
    inline Context(
		FlightInfo *fi,
		AutoPilot *ap,
		Ptr<DroneControls> controls,
	    Ptr<IGame> thegame,
		Ptr<ITerrain> terrain,
	    Ptr<IActor> actor,
	    Ptr<Targeter> targeter,
	    MTasker<> & mtasker)
	:	fi(fi), ap(ap), controls(controls), thegame(thegame),
		terrain(terrain), actor(actor), targeter(targeter),
		mtasker(mtasker)
	{ }
    	
};


struct Idea : virtual public Object {
	Context & ctx;
	std::string name;
	inline Idea(Context & ctx, const std::string & name="Idea")
	: ctx(ctx), name(name) { }
    virtual Rating rate()=0;
    virtual void realize()=0;
    virtual void postpone()=0;
    virtual std::string info()=0;
};

class PatrolIdea : public Idea {
    Vector area;
    std::deque<Vector> path;
    float radius;
public:
    PatrolIdea(
    	Context & ctx,
    	const Vector & area,
        float radius);
    virtual Rating rate();
    virtual void realize();
    virtual void postpone();
    virtual std::string info();
    inline std::deque<Vector> & getPath() { return path; }
protected:
    void choosePath();
};

class AttackIdea : public Idea {
    Ptr<IActor> target;
    Rendezvous rendezvous;
    double last_target_select;
    double last_missile;
    double last_bullet;
    double delta_t;
    Vector d,v,p,front,target_rendezvous;
    float target_dist,target_angle;
public:
    AttackIdea(Context & ctx);
    virtual Rating rate();
    virtual void realize();
    virtual void postpone();
    virtual std::string info();
protected:
    void evaluateSituation();
    bool canFireMissile();
    void fireMissile();
    bool canFireBullet();
    void fireBullet();
    bool canSelectTarget();
    void selectTarget();
    void followTarget();
};

struct EvadeTerrainIdea : public Idea {
	bool triggered;
	inline EvadeTerrainIdea(Context & ctx) : Idea(ctx,"Evade"), triggered(false) { }
    virtual Rating rate();
    virtual void realize();
    virtual void postpone();
    virtual std::string info();
};

class CRIdea : public Idea, public CoRoutine {
    std::string nfo;
public:
	inline CRIdea(Context & ctx)
	:	CoRoutine(ctx.mtasker), Idea(ctx,"CRIdea")
	{ }
    virtual Rating rate();
    virtual void realize();
    virtual void postpone();
    virtual std::string info();
    virtual void run();
};

class Dogfight : public Idea, public CoRoutine {
	std::string nfo;
public:
	inline Dogfight(Context & ctx)
	:	CoRoutine(ctx.mtasker), Idea(ctx, "Dogfight")
	{ }
    virtual Rating rate();
    virtual void realize();
    virtual void postpone();
    virtual std::string info();
    virtual void run();
private:
	bool targetInRange(Ptr<IActor>);
	Ptr<IActor> selectNearestTargetInRange(float range=5000);
	bool positionFavorable();
	void aimInDirection(Vector);
	void aimAndShoot();
	void evade();
	void flyLooping();
	void flyScissors();
	void flyImmelmann();
	void flyImmelmannTurn();
	void flySplitS();
	void gainSpeedAndStabilize();
	void stabilize();
};

#endif
