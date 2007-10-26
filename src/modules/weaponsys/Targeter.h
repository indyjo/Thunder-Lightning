#ifndef TARGETER_H
#define TARGETER_H

#include <vector>
#include <interfaces/IActor.h>
#include <interfaces/IActorStage.h>
#include <interfaces/ITerrain.h>
#include "RadarNet.h"

class Targeter : public SigObject, public Weak {
public:
	Targeter(Ptr<ITerrain>, IActorStage &, IActor &);
	
	void setMaxRange(float);
    inline float getMaxRange() { return max_range; }
	//void setTargetFilter(TargetFilter
	
	inline IActor & getSubjectActor() { return self; }
	
	inline Ptr<RadarNet> getRadarNet() { return radarnet; }
	inline void setRadarNet(Ptr<RadarNet> rn) { radarnet = rn; }
	
	Ptr<IActor> getCurrentTarget();
	void setCurrentTarget(Ptr<IActor> target);
	
	void selectNextTarget();
	void selectPreviousTarget();
	
	void selectNextHostileTarget();
	void selectPreviousHostileTarget();
	
	void selectNextFriendlyTarget();
	void selectPreviousFriendlyTarget();
	
	void selectNearestTarget();
	void selectNearestHostileTarget();
	void selectNearestFriendlyTarget();
	
	void selectTargetNearVector(const Vector & pos, const Vector & dir);
	void selectTargetInGunsight();
	
	void clearCurrentTarget();
	
	void listTargets(std::vector<Ptr<IActor> > & actors);
	void listHostileTargets(std::vector<Ptr<IActor> > & actors);
	void listFriendlyTargets(std::vector<Ptr<IActor> > & actors);
	
	void update(float delta_t);
	
	bool hasLineOfSightTo(Ptr<IActor>);
protected:
	void selectNextFrom(std::vector<Ptr<IActor> > & actors);
	void selectPreviousFrom(std::vector<Ptr<IActor> > & actors);
	void selectNearestFrom(std::vector<Ptr<IActor> > & actors);
	void selectTargetNearVectorFrom(const Vector& pos, const Vector& dir, std::vector<Ptr<IActor> > & actors);

	IActor & self;
	IActorStage & stage;
	float max_range;
	Ptr<ITerrain> terrain;
	Ptr<RadarNet> radarnet;
	RadarNet::Enumerator current;
	float time_since_scan;
};

#endif
