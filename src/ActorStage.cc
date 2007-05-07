#include <algorithm>
#include <interfaces/IActor.h>
#include <ActorStage.h>


void ActorStage::addActor(Ptr<IActor> actor) {
    actors.push_back(actor);
    actor->onLinked();
}

void ActorStage::removeActor(Ptr<IActor> actor) {
    ActorVector::iterator iter = find(actors.begin(), actors.end(), actor);
    if (iter != actors.end()) {
        actors.erase(iter);
        actor->onUnlinked();
    }
}

void ActorStage::addWeakActor(WeakPtr<IActor> actor) {
    if (actor.valid()) weak_actors.push_back(actor);
}

void ActorStage::queryActorsInSphere(ActorVector & out,
                                     const Vector & x, float r)
{
    typedef ActorVector::iterator Iter;
    r *= r;
    for(Iter i=actors.begin(); i!=actors.end(); ++i)
        if(((*i)->getLocation()-x).lengthSquare() <= r)
            out.push_back(*i);
}

void ActorStage::queryActorsInCylinder(ActorVector & out,
                                       const Vector & x, float r)
{
    typedef ActorVector::iterator Iter;
    r *= r;
    Vector y;
    for(Iter i=actors.begin(); i!=actors.end(); ++i) {
        y = x - (*i)->getLocation();
        if(y[0]*y[0]+y[2]*y[2] <= r)
            out.push_back(*i);
    }
}

void ActorStage::queryActorsInBox(ActorVector & out,
                                  const Vector &x_min, const Vector &x_max)
{
    Vector x;
    typedef ActorVector::iterator Iter;
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

void ActorStage::cleanupActors() {
	int removed=0;
	for(int i=0; i<actors.size(); ++i) {
		Ptr<IActor> a=actors[i];
		if (a->getState()==IActor::DEAD) {
		    a->onUnlinked();
			removed++;
		} else if (removed>0) actors[i-removed]=a;
	}
	actors.resize(actors.size()-removed);
	//if (removed>0) ls_message("ActorStage: %d actors removed\n", removed);

    removed=0;
	for(int i=0; i<weak_actors.size(); ++i) {
		WeakPtr<IActor> a=weak_actors[i];
        if (!a.valid()) {
			removed++;
		} else if (removed>0) weak_actors[i-removed]=a;
	}
	weak_actors.resize(weak_actors.size()-removed);
}

void ActorStage::setupActors() {
	for(int i=0; i<actors.size(); ++i)
		actors[i]->action();
	for(int i=0; i<weak_actors.size(); ++i)
        if(weak_actors[i].valid()) weak_actors[i]->action();
}

void ActorStage::drawActors() {
	for(int i=0; i<actors.size(); ++i)
		actors[i]->draw();
	for(int i=0; i<weak_actors.size(); ++i)
        if(weak_actors[i].valid()) weak_actors[i]->draw();
}

void ActorStage::removeAllActors() {
	for(int i=0; i<actors.size(); ++i) {
		actors[i]->kill();
		actors[i]->onUnlinked();
	}
	actors.clear();
    weak_actors.clear();
}

