#ifndef IACTORSTAGE_H
#define IACTORSTAGE_H
#include <list>
#include <object.h>

class IActor;

class IActorStage : virtual public Object
{
public:
    typedef std::list< Ptr<IActor> > ActorList;

    virtual void addActor(Ptr<IActor>)=0;
    virtual const ActorList & getActorList()=0;
    virtual void queryActorsInSphere(ActorList &, const Vector &, float)=0;
    virtual void queryActorsInCylinder(ActorList &, const Vector &, float)=0;
    virtual void queryActorsInBox(ActorList &, const Vector &, const Vector &)=0;
};
#endif
