#ifndef IACTORSTAGE_H
#define IACTORSTAGE_H
#include <list>
#include <vector>
#include <Weak.h>
#include <modules/math/Vector.h>

struct IActor;

struct IActorStage : virtual public Object
{
    typedef std::vector< Ptr<IActor> > ActorVector;

    /// Adds an actor to the actor stage.
    virtual void addActor(Ptr<IActor>)=0;
    /// Removes an actor from the actor stage
    virtual void removeActor(Ptr<IActor>)=0;
    /// Adds an actor to the actor stage by means of a weak reference.
    /// Thus, the actor's lifetime is defined by external references.
    /// As soon as the actor is no longer referenced, it disappears from the scene.
    ///
    /// @note: Weakly referenced actors can not be queried with the queryActorsInX member functions.
    virtual void addWeakActor(WeakPtr<IActor>)=0;

    virtual void queryActorsInSphere(ActorVector &, const Vector &, float)=0;
    virtual void queryActorsInCylinder(ActorVector &, const Vector &, float)=0;
    virtual void queryActorsInBox(ActorVector &, const Vector &, const Vector &)=0;
};
#endif
