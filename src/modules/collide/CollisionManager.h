#ifndef COLLISIONMANAGER_H
#define COLLISIONMANAGER_H

#include <queue>
#include <map>
#include <vector>
#include <tnl.h>
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
    std::map<std::string, Ptr<BoundingGeometry> > bounding_geometries;

public:
	CollisionManager();
	~CollisionManager();

    Ptr<BoundingGeometry> queryGeometry(const std::string & name);

    void add(Ptr<Collidable> c);
    void remove(Ptr<Collidable> c);

    void run(Ptr<IGame> game, float delta_t);
    
    /// Static line intersection test.
    /// Finds the first intersection on a line from a to b.
    ///
    /// @param [in]  a          Start point of line
    /// @param [in]  b          End point of line
    /// @param [out] x          Point where the intersection occured, if any
    /// @param [out] normal     Normal at intersection, if any
    
    /// @note: For this function to work properly, the transforms_0 member
    ///        of the geom_instances must reflect the current transforms for
    ///        all collidables. This means that you may call this function
    ///        only in calls to integrate(delta_t, ...) with delta_t != 0.
    Ptr<Collidable> lineQuery(
        const Vector &a,
        const Vector &b,
        Vector * x=0,
        Vector * normal=0,
        Ptr<Collidable> nocollide=0);
};


} // namespace Collide

#endif
