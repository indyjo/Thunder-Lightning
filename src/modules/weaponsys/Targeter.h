#ifndef TARGETER_H
#define TARGETER_H

#include <vector>
#include <interfaces/IActor.h>
#include <interfaces/IActorStage.h>

class Targeter : public SigObject {
public:
	Targeter(IActorStage &, IActor &);
	
	void setMaxRange(float);
    inline float getMaxRange() { return max_range; }
	//void setTargetFilter(TargetFilter
	
	Ptr<IActor> getCurrentTarget();
	inline void setCurrentTarget(Ptr<IActor> target) { current = target; }
	
	void selectNextTarget();
	void selectPreviousTarget();
	
	void selectNextHostileTarget();
	void selectPreviousHostileTarget();
	
	void selectNextFriendlyTarget();
	void selectPreviousFriendlyTarget();
	
	void selectNearestTarget();
	void selectNearestHostileTarget();
	void selectNearestFriendlyTarget();
	
	void selectTargetInGunsight();
	
	void clearCurrentTarget();
	
	void listTargets(std::vector<Ptr<IActor> > & actors);
	void listHostileTargets(std::vector<Ptr<IActor> > & actors);
	void listFriendlyTargets(std::vector<Ptr<IActor> > & actors);
protected:
	void selectNextFrom(std::vector<Ptr<IActor> > & actors);
	void selectPreviousFrom(std::vector<Ptr<IActor> > & actors);
	void selectNearestFrom(std::vector<Ptr<IActor> > & actors);
	void selectTargetInGunsightFrom(std::vector<Ptr<IActor> > & actors);

	IActor & self;
	IActorStage & stage;
	Ptr<IActor> current;
	float max_range;
};

#endif
