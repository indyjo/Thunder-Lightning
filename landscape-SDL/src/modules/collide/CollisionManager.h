#ifndef COLLISIONMANAGER_H
#define COLLISIONMANAGER_H

#include <queue>
#include <map>
#include <vector>
#include <landscape.h>
#include <modules/physics/RigidBody.h>
#include <modules/math/Transform.h>
#include "BoundingBox.h"
#include "SweepNPrune.h"
#include <interfaces/IActor.h>
#include <interfaces/IGame.h>


namespace Collide {

struct Contact;
struct PossibleContact;
class Collidable;
class CollisionManager;

class Collidable : virtual public Object{
    Ptr<BoundingGeometry> bounding;
    Ptr<IActor> actor;
    Ptr<RigidBody> rigid;
protected:
    inline Collidable(Ptr<BoundingGeometry> b=0,
                      Ptr<RigidBody> r=0, Ptr<IActor> a=0)
    :   bounding(b), rigid(r), actor(a)
    { }
protected:
    inline void setBoundingGeometry(Ptr<BoundingGeometry> b) {
        this->bounding = b;
    }
    inline void setRigidBody(Ptr<RigidBody> r) {
        this->rigid = r;
    }
    inline void setActor(Ptr<IActor> a) {
        this->actor = a;
    }
public:
    // returns associated bounding geometry.
    inline Ptr<BoundingGeometry>
    getBoundingGeometry() { return bounding; }

    // returns an associated rigid body.
    // Warning: this can be null!
    inline Ptr<RigidBody> getRigid() { return rigid; }

    // returns an associated actor.
    // Warning: This can be null!
    inline Ptr<IActor> getActor() { return actor; }

    // These three methods have to be implemented by the derived class

    // Query the state of the transforms after delta_t seconds.
    // If delta_t == 0 this just copies the current state
    // negative delta_t values are forbidden
    virtual void integrate(float delta_t, Transform * transforms) = 0;

    // Update the state of the object and signal that delta_t seconds have passed
    // since the last update.
    virtual void update(float delta_t, const Transform * new_transforms) = 0;

    // Signal the object that a collision is about to happen.
    // The purpose of this method is not to calculate the rigid body collision response
    // (i.e. an impulse) but for the game logic to afflict damage to the partner or
    // score points and so on.
    // The default implementation is to do nothing
    virtual void collide(const Contact &);
};

struct Contact {
    Ptr<Collidable> collidables[2];
    int domains[2];
    // position and normal of collision.
    // The normal is pointing towards collidables[0]
    Vector p, n;
    // velocity of the objects wrt normal.
    // v[0] should be negative, v[1] positive
    float v[2];

    void applyCollisionImpulse();
};

struct GeometryInstance {
    Ptr<Collidable> collidable;
    Transform * transforms_0; // The state at the beginning and the end of the
    Transform * transforms_1; // active time interval

    inline GeometryInstance()
    {
        collidable = 0;
        transforms_0 = new Transform[0];
        transforms_1 = new Transform[0];
    }

    inline GeometryInstance(Ptr<Collidable> c)
    :   collidable(c)
    {
        int n = c->getBoundingGeometry()->getNumOfTransforms();
        transforms_0 = new Transform[n];
        transforms_1 = new Transform[n];
    }

    inline GeometryInstance(const GeometryInstance & g)
    :   collidable(g.collidable)
    {
        int n;
        if(collidable) {
            n = collidable->getBoundingGeometry()->getNumOfTransforms();
        } else n = 0;
        transforms_0 = new Transform[n];
        transforms_1 = new Transform[n];
        for(int i=0; i<n; i++) {
            transforms_0[i] = g.transforms_0[i];
            transforms_1[i] = g.transforms_1[i];
        }
    }

    inline ~GeometryInstance() {
        delete [] transforms_0;
        delete [] transforms_1;
    }

    inline GeometryInstance & operator=(const GeometryInstance & g)
    {
        delete [] transforms_0;
        delete [] transforms_1;

        collidable = g.collidable;
        int n;
        if(collidable) {
            n = collidable->getBoundingGeometry()->getNumOfTransforms();
        } else n = 0;
        transforms_0 = new Transform[n];
        transforms_1 = new Transform[n];
        for(int i=0; i<n; i++) {
            transforms_0[i] = g.transforms_0[i];
            transforms_1[i] = g.transforms_1[i];
        }
        
        return *this;
    }
};

struct ContactPartner {
    GeometryInstance * instance;
    int domain, transform;
    enum { TRIANGLE, NODE, GEOM } type;
    union {
        const Vector           * triangle;
        const BoundingNode     * node;
        const BoundingGeometry * geom;
    } data;

    inline ContactPartner() { }

    inline ContactPartner(GeometryInstance * g, Vector * t, int dom=0, int tr=0)
    :   instance(g), type(TRIANGLE),
        domain(dom), transform(tr)
    {
        data.triangle = t;
    }

    inline ContactPartner(GeometryInstance * g, BoundingNode * n, int dom=0, int tr=0)
    :   instance(g), type(NODE),
        domain(dom), transform(tr)
    {
        data.node = n;
    }

    inline ContactPartner(GeometryInstance * g, BoundingGeometry * b, int dom=0, int tr=0)
    :   instance(g), type(GEOM),
        domain(dom), transform(tr)
    {
        data.geom = b;
    }

    inline bool isTriangle() { return type == TRIANGLE; }
    inline bool isBox() { return type == NODE; }
    inline bool isSphere() { return type == GEOM; }
    inline bool canSubdivide() {
        if (isTriangle()) return false;
        else if (isBox()) return data.node->type != BoundingNode::NONE;
        else return data.geom->getRootNode()->type != BoundingNode::NONE;
    }
};

union Hints; // defined in Primitive.h

struct PossibleContact {
    ContactPartner partners[2];
    float t0, t1;

    inline friend bool operator < (const PossibleContact & a,
                                   const PossibleContact & b) {
        // a has lower priority than b if:
        return b.t0 < a.t0 || b.t0 == a.t0 && b.t1 < a.t1;
    }

    inline bool canSubdivide() {
        return partners[0].canSubdivide() || partners[1].canSubdivide();
    }
    bool mustSubdivide();
    void subdivide(std::priority_queue<PossibleContact> & q);
    bool shouldDivideTime(const Hints & hints);
    void divideTime(std::priority_queue<PossibleContact> & q);
    bool collide(float delta_t, Hints & hints);
    bool makeContact(Contact & c, float delta_t, const Hints & hints);
};

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
    Ptr<BoundingGeometry> queryGeometry(const std::string & name);

    void add(Ptr<Collidable> c);
    void remove(Ptr<Collidable> c);

    void run(Ptr<IGame> game, float delta_t);
};


} // namespace Collide

#endif
