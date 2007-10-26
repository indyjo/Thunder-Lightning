#include <algorithm>
#include <Faction.h>
#include <TargetInfo.h>
#include "RadarNet.h"
#include "Targeter.h"

using namespace std;

Targeter::Targeter(Ptr<ITerrain> terrain, IActorStage &stage, IActor &self)
:	self(self),
	stage(stage),
	max_range(10000),
	terrain(terrain),
	radarnet(new RadarNet),
	time_since_scan(0),
	current(radarnet->getEnumerator())
{
    current.toEnd();
}

void Targeter::setMaxRange(float r) {
	max_range=r;
}

void Targeter::setCurrentTarget(Ptr<IActor> target) {
    current = radarnet->getEnumeratorForActor(target);
}

Ptr<IActor> Targeter::getCurrentTarget() {
	return current.getActor();
}

void Targeter::selectNextTarget() {
	current.cycle();
}
void Targeter::selectPreviousTarget() {
    current.cycle(true);
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
	current.toEnd();
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
    typedef RadarNet::Enumerator Enum;
    for( Enum e = radarnet->getEnumerator(); !e.atEnd(); e.next()) {
        if (e.getActor()) actors.push_back(e.getActor());
    }
    vector<Ptr<IActor> >::iterator i = 
    	find(actors.begin(), actors.end(), Ptr<IActor>(&self));
	if (i!=actors.end()) actors.erase(i);
	actors.resize(remove_if(actors.begin(),actors.end(),invalid)
	              - actors.begin());
}

void Targeter::listHostileTargets(vector<Ptr<IActor> > & actors) {
    typedef RadarNet::Enumerator Enum;
    for( Enum e = radarnet->getEnumerator(); !e.atEnd(); e.next()) {
        if (e.getActor()) actors.push_back(e.getActor());
    }
    vector<Ptr<IActor> >::iterator i = 
    	find(actors.begin(), actors.end(), Ptr<IActor>(&self));
	if (i!=actors.end()) actors.erase(i);
	
	Ptr<Faction> faction=self.getFaction();
	
	actors.resize(remove_if(actors.begin(),actors.end(),not_hostile(faction))
	              - actors.begin());
}

void Targeter::listFriendlyTargets(vector<Ptr<IActor> > & actors) {
    typedef RadarNet::Enumerator Enum;
    for( Enum e = radarnet->getEnumerator(); !e.atEnd(); e.next()) {
        if (e.getActor()) actors.push_back(e.getActor());
    }
    vector<Ptr<IActor> >::iterator i = 
    	find(actors.begin(), actors.end(), Ptr<IActor>(&self));
	if (i!=actors.end()) actors.erase(i);
	
	Ptr<Faction> faction=self.getFaction();
	
	actors.resize(remove_if(actors.begin(),actors.end(),not_friendly(faction))
	              - actors.begin());
}

void Targeter::update(float delta_t) {
    radarnet->reportSelf(this);
    
    time_since_scan += delta_t;
    const float TIME_TO_SCAN = 2.0f;
    if (time_since_scan > TIME_TO_SCAN) {
        typedef std::vector<Ptr<IActor> > Actors;
        Actors actors;
	    stage.queryActorsInSphere(
		    actors,
		    self.getLocation(),
		    max_range<0?1e15:max_range);
		for(Actors::iterator i=actors.begin(); i!= actors.end(); ++i) {
		    if (invalid(*i)) continue;
		    radarnet->reportPossibleContact(*i, this);
		}
		    
        time_since_scan -= TIME_TO_SCAN;
    }
}

bool Targeter::hasLineOfSightTo(Ptr<IActor> target) {
    Vector x;
    return !terrain->lineCollides(self.getLocation(), target->getLocation(), &x);
}

void Targeter::selectNextFrom(vector<Ptr<IActor> > & actors) {
	Ptr<IActor> target = current.getActor();
	vector<Ptr<IActor> >::iterator i=
		target?find(actors.begin(), actors.end(), target)
		       :actors.end();
	if (i==actors.end()) i=actors.begin();
	else ++i;
	if (i == actors.end()) {
	    current.toEnd();
	} else {
	    current = radarnet->getEnumeratorForActor(*i);
	}
}

void Targeter::selectPreviousFrom(vector<Ptr<IActor> > & actors) {
	Ptr<IActor> target = current.getActor();
	vector<Ptr<IActor> >::iterator i=
		target?find(actors.begin(), actors.end(), target)
		       :actors.end();
	if (i!=actors.begin()) --i;
	else if (i!=actors.end()) i=actors.end()-1;
	if (i == actors.end()) {
	    current.toEnd();
	} else {
	    current = radarnet->getEnumeratorForActor(*i);
	}
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
		current.toEnd();
		return;
	}
	current= radarnet->getEnumeratorForActor(
	    *min_element( actors.begin(),actors.end(),
                      NearerTo(self.getLocation())));
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
		current.toEnd();
		return;
	}
	current= radarnet->getEnumeratorForActor(
	    *max_element( actors.begin(),actors.end(),
                      MoreInFrontOf(pos,dir)));
}

