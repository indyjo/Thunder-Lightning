#include <algorithm>
#include <Faction.h>
#include <TargetInfo.h>
#include "Targeter.h"

using namespace std;

Targeter::Targeter(IActorStage &stage, IActor &self)
:	self(self),
	stage(stage),
	max_range(-1)
{
}

void Targeter::setMaxRange(float r) {
	max_range=r;
}

Ptr<IActor> Targeter::getCurrentTarget() {
	if (current&&current->getState()==IActor::DEAD)
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
	selectTargetInGunsightFrom(actors);
}	

	
void Targeter::listTargets(vector<Ptr<IActor> > & actors) {
	stage.queryActorsInCylinder(
		actors,
		self.getLocation(),
		max_range<0?1e15:max_range);
	actors.erase(find(actors.begin(), actors.end(), Ptr<IActor>(&self)));
	int removed=0;
	for(int i=0;i<actors.size()-removed;++i) {
		if (!actors[i]->getTargetInfo() ||
			!actors[i]->getTargetInfo()->isA(
				TargetInfo::DETECTABLE))
		{
			swap(actors[i],actors[actors.size()-1-removed]);
			++removed;
			--i;
		}
	}
	actors.resize(actors.size()-removed);
}

void Targeter::listHostileTargets(vector<Ptr<IActor> > & actors) {
	stage.queryActorsInCylinder(
		actors,
		self.getLocation(),
		max_range<0?1e15:max_range);
	actors.erase(find(actors.begin(), actors.end(), Ptr<IActor>(&self)));
	
	Ptr<Faction> faction=self.getFaction();
	
	int removed=0;
	for(int i=0;i<actors.size()-removed;++i) {
		Faction::Attitude attitude =
			faction->getAttitudeTowards(actors[i]->getFaction());
		if (attitude != Faction::HOSTILE) {
			swap(actors[i],actors[actors.size()-1-removed]);
			++removed;
			--i;
		} else if (!actors[i]->getTargetInfo() ||
			!actors[i]->getTargetInfo()->isA(
				TargetInfo::DETECTABLE))
		{
			swap(actors[i],actors[actors.size()-1-removed]);
			++removed;
			--i;
		}
	}
	actors.resize(actors.size()-removed);
}

void Targeter::listFriendlyTargets(vector<Ptr<IActor> > & actors) {
	stage.queryActorsInCylinder(
		actors,
		self.getLocation(),
		max_range<0?1e15:max_range);
	actors.erase(find(actors.begin(), actors.end(), Ptr<IActor>(&self)));
	
	Ptr<Faction> faction=self.getFaction();
	
	int removed=0;
	for(int i=0;i<actors.size()-removed;++i) {
		Faction::Attitude attitude =
			faction->getAttitudeTowards(actors[i]->getFaction());
		if (attitude != Faction::FRIENDLY) {
			swap(actors[i],actors[actors.size()-1-removed]);
			++removed;
			--i;
		} else if (!actors[i]->getTargetInfo() ||
			!actors[i]->getTargetInfo()->isA(
				TargetInfo::DETECTABLE))
		{
			swap(actors[i],actors[actors.size()-1-removed]);
			++removed;
			--i;
		}
	}
	actors.resize(actors.size()-removed);
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
void Targeter::selectTargetInGunsightFrom(vector<Ptr<IActor> > & actors) {
	if(actors.empty()) {
		current=0;
		return;
	}
	current=*max_element(
		actors.begin(),actors.end(),
		MoreInFrontOf(self.getLocation(),self.getFrontVector()));
}

