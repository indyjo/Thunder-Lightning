#ifndef ACTORSTAGE_H
#define ACTORSTAGE_H


#include <list>
#include <landscape.h>

class ActorStage : virtual public IActorStage
{
protected:
    ActorList actors;
public:
    virtual void addActor(Ptr<IActor>);
    virtual const ActorList & getActorList();
    virtual void queryActorsInSphere(ActorList &, const Vector &, float);
    virtual void queryActorsInCylinder(ActorList &, const Vector &, float);
    virtual void queryActorsInBox(ActorList &, const Vector &, const Vector &);
};

#endif
