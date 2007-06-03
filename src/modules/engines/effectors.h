#ifndef EFFECTORS_H
#define EFFECTORS_H

#include <interfaces/IEffector.h>
#include <interfaces/ITerrain.h>
#include <modules/math/Vector.h>
#include "controls.h"

namespace Effectors {

class Gravity: public IEffector {
    // private constructor to force usage of singleton
    Gravity();
public:
    virtual void applyEffect(RigidBody &rigid);
    
    /// Singleton accessor function
    static Ptr<Gravity> getInstance();
private:
    static Ptr<Gravity> singleton;
};



class Flight : public IEffector {
    Ptr<FlightControls> controls;
public:
    Flight(Ptr<FlightControls> controls) : controls(controls) { }
    
    virtual void applyEffect(RigidBody &rigid);
};



class Wheel : public IEffector {
public:
    struct Params {
    	Vector pos, axis;
    	float range, force, damping, drag_long, drag_lat;
    };
    
    Wheel(Ptr<ITerrain> terrain, const Params & params)
        : terrain(terrain)
    { setParams(params); }
    
    const Params & getParams() { return params; }
    const Vector & getCurrentPos() { return current_pos; }
    const float  getCurrentLoad() { return current_load; }
    const bool hasContact() { return contact; }
    
    void setParams(const Params&);
    
    virtual void applyEffect(RigidBody &rigid);
    
private:
    Ptr<ITerrain> terrain;
    Params params;
    
    // dynamic state
    Vector current_pos;
    float current_load;
    bool contact;
};    


} // namespace Effectors

#endif

