#ifndef EFFECTORS_H
#define EFFECTORS_H

#include <interfaces/IEffector.h>
#include <interfaces/ITerrain.h>
#include <modules/math/Vector.h>
#include <modules/model/model.h>
#include <Weak.h>
#include "controls.h"

namespace Collide {
    class CollisionManager;
    class Collidable;
}


class RigidEngine;



namespace Effectors {

class Gravity: public IEffector {
    // private constructor to force usage of singleton
    Gravity();
public:
    virtual void applyEffect(RigidBody &rigid, Ptr<DataNode> controls);
    
    /// Singleton accessor function
    static Ptr<Gravity> getInstance();
private:
    static Ptr<Gravity> singleton;
};



class Flight : public IEffector {
    Ptr<FlightControls> controls;
public:
    Flight(Ptr<FlightControls> controls) : controls(controls) { }
    
    virtual void applyEffect(RigidBody &rigid, Ptr<DataNode> controls);
};



class Wheel : public IEffector {
public:
    struct Params {
    	Vector pos, axis;
    	float range, force, damping, drag_long, drag_lat;
    };
    
    Wheel(Ptr<ITerrain> terrain, Ptr<Collide::CollisionManager> cm, WeakPtr<Collide::Collidable> nocollide, const Params & params);
    
    Params & getParams() { return params; }
    const Vector & getCurrentPos() { return current_pos; }
    const float  getCurrentLoad() { return current_load; }
    const bool hasContact() { return contact; }
    
    void setParams(const Params&);
    
    virtual void applyEffect(RigidBody &rigid, Ptr<DataNode> controls);
    
private:
    Ptr<ITerrain> terrain;
    Ptr<Collide::CollisionManager> collision_manager;
    WeakPtr<Collide::Collidable> nocollide;
    Params params;
    
    // dynamic state
    Vector current_pos;
    float current_load;
    bool contact;
};    


class Thrust : public IEffector {
    Vector max_force;
    float throttle;
    
public:
    Thrust();
    
    inline void setMaxForce(Vector f) { max_force = f; }
    inline Vector getMaxForce() { return max_force; }
    
    inline void setThrottle(float t) { throttle = t; }
    inline float getThrottle() { return throttle; }
    
    inline Vector getEffectiveForce() { return throttle*max_force; }

    virtual void applyEffect(RigidBody &rigid, Ptr<DataNode> controls);
};

class Missile : public IEffector {
public:
    virtual void applyEffect(RigidBody &rigid, Ptr<DataNode> controls);
};

class MissileControl : public IEffector {
public:
    virtual void applyEffect(RigidBody &rigid, Ptr<DataNode> controls);
};


class Buoyancy : public IEffector {
    Vector p, n;
    float area;
public:
    inline Buoyancy(Vector p, Vector n, float area)
    : p(p), n(n), area(area)
    { }
    
    static void addBuoyancyFromMesh(Ptr<RigidEngine>, Ptr<Model::Group>, Ptr<Model::MeshData>, Vector offset=Vector(0,0,0));
    static void addBuoyancyFromMesh(Ptr<RigidEngine>, Ptr<Model::Object>, Vector offset=Vector(0,0,0));
    
    virtual void applyEffect(RigidBody &rigid, Ptr<DataNode> controls);
};


class TailHook : public IEffector {
    Vector p0, p1;
    float max_force;
    Ptr<RigidBody> partner;
    WeakPtr<Collide::Collidable> nocollide;
    Ptr<Collide::CollisionManager> collision_manager;

public:
    inline TailHook(Ptr<Collide::CollisionManager> cm, WeakPtr<Collide::Collidable> nocollide, Vector p0, Vector p1, float max_force)
    : nocollide(nocollide), p0(p0), p1(p1), max_force(max_force), collision_manager(cm)
    { }
    
    virtual void applyEffect(RigidBody &rigid, Ptr<DataNode> controls);
    inline void clear() { partner = 0; }
};

} // namespace Effectors

#endif

