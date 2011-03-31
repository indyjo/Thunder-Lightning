#ifndef EFFECTORS_H
#define EFFECTORS_H

#include <string>
#include <interfaces/IConfig.h>
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

/// Effector that will provide a simple drag force contrary to motion vector
class Drag: public IEffector {
    float CdA;
public:
    /// Initialize with Cd times A value
    Drag(float CdA=1.0);
    
    virtual void applyEffect(RigidBody &rigid, Ptr<DataNode> controls);
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

/// An effector simulating a wheel spun with constant velocity,
/// without taking forces into account that act on the wheels rotational speed.
class SpinningWheel : public IEffector {
public:
    struct Params {
        /// Position of wheel in relaxed state in LCS
    	Vector pos;
    	/// Spring column direction (the direction in which the spring is compressed) in LCS.
    	/// Must be unit-sized
    	Vector spring;
    	/// The axis which is aligned with the wheel's axle. Should be orthogonal to spring.
    	Vector axle;
    	/// Speed of wheel rotation, measured at its point of contact with the ground.
    	/// Positive values rotate counter-clockwise around the axle vector.
        float spin;
    	/// Length, maximum force and damping of the wheel's spring
    	float length, force, damping;
    	/// Friction F/delta_v between wheel and terrain in N / (m/s)
    	float friction;
    	/// Friction under water.
    	float friction_under_water;
    } params;
    
    SpinningWheel(Ptr<ITerrain> terrain, Ptr<Collide::CollisionManager> cm, WeakPtr<Collide::Collidable> nocollide);
    virtual void applyEffect(RigidBody &rigid, Ptr<DataNode> controls);

    const Vector & getCurrentPos() { return current_pos; }
    const Vector & getCurrentFriction() { return current_friction; }
    const float  getCurrentLoad() { return current_load; }
    const bool hasContact() { return contact; }
    
    inline void setDebug(bool b) { debug = b; }
    
private:
    Ptr<ITerrain> terrain;
    Ptr<Collide::CollisionManager> collision_manager;
    WeakPtr<Collide::Collidable> nocollide;
    
    // dynamic state
    Vector current_pos;
    Vector current_friction;
    float current_load;
    bool contact;
    bool debug;
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
    float CdA_f;             // Frontal area times frontal drag coefficient
    float CdA_s;             // Area of side times drag coefficient of side
    float torque_factor_z;
    float torque_factor_xy;
    float pitching_factor;
    
    Missile(Ptr<IConfig> cfg, const std::string & prefix);
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

