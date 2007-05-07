#ifndef FLIGHTENGINE2_H
#define FLIGHTENGINE2_H

#include <modules/engines/rigidengine.h>
#include <modules/engines/controls.h>

class FlightEngine2: public RigidEngine {
public:
    FlightEngine2(
        Ptr<IGame> game,
        float max_thrust = 40000.0,
        float wing_area = 20.0,
        float max_torque = 600000.0f);

    inline Ptr<FlightControls> getFlightControls() { return controls; }

    inline float getMaxThrust() { return max_thrust; }
    inline void setMaxThrust(float new_thrust) { max_thrust = new_thrust; }

    //IEngine
    virtual void setControls(Ptr<DataNode> controls);
    virtual void run();

protected:
    Ptr<FlightControls> controls;

    // static parameters
    float max_thrust, max_torque, wing_area, mass;
};

#endif
