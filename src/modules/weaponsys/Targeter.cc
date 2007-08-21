#include <algorithm>
#include <Faction.h>
#include <TargetInfo.h>
#include "Targeter.h"

using namespace std;

Targeter::Targeter(Ptr<ITerrain> terrain, IActorStage &stage, IActor &self)
:	self(self),
	stage(stage),
	max_range(10000),
	terrain(terrain)
{
}

void Targeter::setMaxRange(float r) {
	max_range=r;
}

Ptr<IActor> Targeter::getCurrentTarget() {
	if (current&& !current->isAlive())
		current=0;
	return current;
}

void Targeter::selectNextTarget() {
	vector<Ptr<IActor> > actors;
	listTargets(actors);
	selectNextFrom(actors);
}
void Targeter::selectPreviousTarget() {
	vector<Ptr<IActor> > actors;
	listTargets(actors);
	selectPreviousFrom(actors);
}

void Targeter::selectNextHostileTarget() {
	vector<Ptr<IActor> > actors;
	listHostileTargets(actors);
	selectNextFrom(actors);
}
void Targeter::selectPreviousHostileTarget() {
	vector<Ptr<IActor> > actors;
	listHostileTargets(actors);
	selectPreviousFrom(actors);
}

void Targeter::selectNextFriendlyTarget() {
	vector<Ptr<IActor> > actors;
	listFriendlyTargets(actors);
	selectNextFrom(actors);
}
void Targeter::selectPreviousFriendlyTarget() {
	vector<Ptr<IActor> > actors;
	listFriendlyTargets(actors);
	selectPreviousFrom(actors);
}

void Targeter::selectNearestTarget() {
	vector<Ptr<IActor> > actors;
	listTargets(actors);
	selectNearestFrom(actors);
}

void Targeter::selectNearestHostileTarget() {
	vector<Ptr<IActor> > actors;
	listHostileTargets(actors);
	selectNearestFrom(actors);
}

void Targeter::selectNearestFriendlyTarget() {
	vector<Ptr<IActor> > actors;
	listFriendlyTargets(actors);
	selectNearestFrom(actors);
}

void Targeter::selectTargetInGunsight() {
	vector<Ptr<IActor> > actors;
	listTargets(actors);
	selectTargetNearVectorFrom(self.getLocation(), self.getFrontVector(), actors);
}	

void Targeter::selectTargetNearVector(const Vector & pos, const Vector & dir) {
	vector<Ptr<IActor> > actors;
	listTargets(actors);
	selectTargetNearVectorFrom(pos, dir, actors);
}

void Targeter::clearCurrentTarget() {
	current = 0;
}

namespace {

bool invalid(Ptr<IActor> & a) {
	return ! a->getTargetInfo() ||
	       ! a->getTargetInfo()->isA(TargetInfo::DETECTABLE);
}

struct not_friendly {
	Ptr<Faction> faction;
	not_friendly(Ptr<Faction> f) : faction(f) { }
	bool operator () (Ptr<IActor> & a) {
		return invalid(a) || 
		       faction->getAttitudeTowards(a->getFaction())
		           != Faction::FRIENDLY;
	}
};

struct not_hostile {
	Ptr<Faction> faction;
	not_hostile(Ptr<Faction> f) : faction(f) { }
	bool operator () (Ptr<IActor> & a) {
		return invalid(a) || 
		       faction->getAttitudeTowards(a->getFaction())
		           != Faction::HOSTILE;
	}
};

} // namespace
	
void Targeter::listTargets(vector<Ptr<IActor> > & actors) {
	stage.queryActorsInCylinder(
		actors,
		self.getLocation(),
		max_range<0?1e15:max_range);
    vector<Ptr<IActor> >::iterator i = 
    	find(actors.begin(), actors.end(), Ptr<IActor>(&self));
	if (i!=actors.end()) actors.erase(i);
	actors.resize(remove_if(actors.begin(),actors.end(),invalid)
	              - actors.begin());
}

void Targeter::listHostileTargets(vector<Ptr<IActor> > & actors) {
	stage.queryActorsInCylinder(
		actors,
		self.getLocation(),
		max_range<0?1e15:max_range);
    vector<Ptr<IActor> >::iterator i = 
    	find(actors.begin(), actors.end(), Ptr<IActor>(&self));
	if (i!=actors.end()) actors.erase(i);
	
	Ptr<Faction> faction=self.getFaction();
	
	actors.resize(remove_if(actors.begin(),actors.end(),not_hostile(faction))
	              - actors.begin());
}

void Targeter::listFriendlyTargets(vector<Ptr<IActor> > & actors) {
	stage.queryActorsInCylinder(
		actors,
		self.getLocation(),
		max_range<0?1e15:max_range);
    vector<Ptr<IActor> >::iterator i = 
    	find(actors.begin(), actors.end(), Ptr<IActor>(&self));
	if (i!=actors.end()) actors.erase(i);
	
	Ptr<Faction> faction=self.getFaction();
	
	actors.resize(remove_if(actors.begin(),actors.end(),not_friendly(faction))
	              - actors.begin());
}

bool Targeter::hasLineOfSightTo(Ptr<IActor> target) {
    Vector x;
    return terrain->lineCollides(self.getLocation(), target->getLocation(), &x);
}

void Targeter::selectNextFrom(vector<Ptr<IActor> > & actors) {
	vector<Ptr<IActor> >::iterator i=
		current?find(actors.begin(), actors.end(), current)
		       :actors.end();
	if (i==actors.end()) i=actors.begin();
	else ++i;
	current = (i==actors.end())?Ptr<IActor>():*i;
}

void Targeter::selectPreviousFrom(vector<Ptr<IActor> > & actors) {
	vector<Ptr<IActor> >::iterator i=
		current?find(actors.begin(), actors.end(), current)
		       :actors.end();
	if (i!=actors.begin()) --i;
	else if (i!=actors.end()) i=actors.end()-1;
	current = (i==actors.end())?Ptr<IActor>():*i;
}

namespace {
	struct NearerTo {
		Vector p;
		NearerTo(const Vector &p) : p(p) {}
		bool operator() (Ptr<IActor> a, Ptr<IActor> b) {
			return (a->getLocation()-p).length()
				<  (b->getLocation()-p).length();
		}
	};
}
void Targeter::selectNearestFrom(vector<Ptr<IActor> > & actors) {
	if(actors.empty()) {
		current=0;
		return;
	}
	current=*min_element(
		actors.begin(),actors.end(),
		NearerTo(self.getLocation()));
}


namespace {
	struct MoreInFrontOf {
		Vector p,d;
		MoreInFrontOf(const Vector &p, const Vector &d) : p(p),d(d) { }
		bool operator() (Ptr<IActor> a, Ptr<IActor> b) {
			return (a->getLocation()-p).normalize()*d
				<  (b->getLocation()-p).normalize()*d;
		}
	};
}
void Targeter::selectTargetNearVectorFrom(const Vector& pos, const Vector& dir, vector<Ptr<IActor> > & actors) {
	if(actors.empty()) {
		current=0;
		return;
	}
	current=*max_element(
		actors.begin(),actors.end(),
		MoreInFrontOf(pos,dir));
}

