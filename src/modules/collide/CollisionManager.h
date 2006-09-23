#ifndef COLLISIONMANAGER_H
#define COLLISIONMANAGER_H

#include <queue>
#include <map>
#include <vector>
#include <landscape.h>
#include <modules/physics/RigidBody.h>
#include <modules/math/Transform.h>
#include "BoundingBox.h"
#include "Collidable.h"
#include "Contact.h"
#include "PossibleContact.h"
#include "SweepNPrune.h"
#include <interfaces/IActor.h>
#include <interfaces/IGame.h>


namespace Collide {

class BoundingGeometry;
class Collidable;
struct Contact;
struct GeometryInstance;
struct PossibleContact;
union Hints; // defined in Primitive.h


class CollisionManager : virtual public Object {
    typedef std::map<Ptr<Collidable>, GeometryInstance *> GeomInstances;
    typedef GeomInstances::iterator GeomIter;
    typedef Ptr<Collidable> Key;
    typedef std::vector<std::pair<Key, Key> > ContactList;
    typedef ContactList::iterator ContactIter;

    GeomInstances geom_instances;
    std::priority_queue<PossibleContact> queue;
    SweepNPrune<Ptr<Collidable>, float> sweep_n_prune;
    ContactList possible_contacts;
    std::map<std::string, Ptr<BoundingGeometry> > bounding_geometries;

public:
	CollisionManager();
	~CollisionManager();

    Ptr<BoundingGeometry> queryGeometry(const std::string & name);

    void add(Ptr<Collidable> c);
    void remove(Ptr<Collidable> c);

    void run(Ptr<IGame> game, float delta_t);
};


} // namespace Collide

#endif
