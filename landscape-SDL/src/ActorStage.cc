#include <interfaces/IActor.h>
#include <ActorStage.h>


void ActorStage::addActor(Ptr<IActor> actor) {
    actors.insert(actors.end(), actor);
}

const IActorStage::ActorList & ActorStage::getActorList() {
    return actors;
}

void ActorStage::queryActorsInSphere(IActorStage::ActorList & out,
                                     const Vector & x, float r)
{
    typedef ActorList::iterator Iter;
    r *= r;
    for(Iter i=actors.begin(); i!=actors.end(); ++i)
        if(((*i)->getLocation()-x).lengthSquare() <= r)
            out.push_back(*i);
}

void ActorStage::queryActorsInCylinder(IActorStage::ActorList & out,
                                       const Vector & x, float r)
{
    typedef ActorList::iterator Iter;
    r *= r;
    Vector y;
    for(Iter i=actors.begin(); i!=actors.end(); ++i) {
        y = (*i)->getLocation();
        if(y[0]*y[0]+y[2]*y[2] <= r)
            out.push_back(*i);
    }
}

void ActorStage::queryActorsInBox(IActorStage::ActorList & out,
                                  const Vector &x_min, const Vector &x_max)
{
    Vector x;
    typedef ActorList::iterator Iter;
    for(Iter i=actors.begin(); i!=actors.end(); ++i) {
        x = (*i)->getLocation();
        if( x[0] >= x_min[0] && x[0] <= x_max[0] &&
            x[1] >= x_min[1] && x[1] <= x_max[1] &&
            x[2] >= x_min[2] && x[2] <= x_max[2])
        {
            out.push_back(*i);
        }
    }
}
