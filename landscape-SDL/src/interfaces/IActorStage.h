#ifndef IACTORSTAGE_H
#define IACTORSTAGE_H
#include <list>
#include <vector>
#include <object.h>
#include <modules/math/Vector.h>

class IActor;

class IActorStage : virtual public Object
{
public:
    typedef std::vector< Ptr<IActor> > ActorVector;

    virtual void addActor(Ptr<IActor>)=0;
    virtual void removeActor(Ptr<IActor>)=0;
    virtual void queryActorsInSphere(ActorVector &, const Vector &, float)=0;
    virtual void queryActorsInCylinder(ActorVector &, const Vector &, float)=0;
    virtual void queryActorsInBox(ActorVector &, const Vector &, const Vector &)=0;
};
#endif
