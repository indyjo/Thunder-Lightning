#ifndef FLIGHTENGINE3_H
#define FLIGHTENGINE3_H

#include <vector>
#include <modules/engines/rigidengine.h>
#include <modules/engines/controls.h>

struct AirFoil {
    Vector position;
    Vector front, up;
    float area;
    float Cd_min, Cd_max;
    float Cl_min, Cl_max;
    Vector rot_axis;
    float rot_factor;
    std::string rot_param;
};

class FlightEngine3: public RigidEngine {
public:
    FlightEngine3(
        Ptr<IGame> game,
        float max_thrust = 40000.0);

    inline Ptr<FlightControls> getFlightControls() { return controls; }

    //IEngine
    virtual void setControls(Ptr<DataNode> controls);
    virtual void run();

protected:
    std::vector<AirFoil> airfoils;
    Ptr<FlightControls> controls;

    // static parameters
    float max_thrust;
};

#endif
