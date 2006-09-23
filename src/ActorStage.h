#ifndef ACTORSTAGE_H
#define ACTORSTAGE_H


#include <list>
#include <landscape.h>
#include <interfaces/IActorStage.h>

class ActorStage : virtual public IActorStage
{
protected:
    ActorVector actors;
    typedef std::vector<WeakPtr<IActor> > WeakActorVector;
    WeakActorVector weak_actors;
public:
    virtual void addActor(Ptr<IActor>);
    virtual void removeActor(Ptr<IActor>);

    virtual void addWeakActor(WeakPtr<IActor>);

    virtual void queryActorsInSphere(ActorVector &, const Vector &, float);
    virtual void queryActorsInCylinder(ActorVector &, const Vector &, float);
    virtual void queryActorsInBox(ActorVector &, const Vector &, const Vector &);
    
    void cleanupActors();
    void setupActors();
    void drawActors();
    void removeAllActors();
    
};

#endif
