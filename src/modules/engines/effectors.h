#ifndef EFFECTORS_H
#define EFFECTORS_H

#include <interfaces/IEffector.h>
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

} // namespace Effectors

#endif

