#ifndef FLIGHTENGINE_H
#define FLIGHTENGINE_H

#include <tnl.h>
#include <modules/engines/controls.h>
#include <interfaces/IGame.h>

#include "engine.h"

class FlightEngine: virtual public IEngine {
public:
    FlightEngine(
        Ptr<IGame> game,
        float max_thrust = 15.0,
        float wing_area = 20.0,
        float mass = 2000.0,
        Vector p0 = Vector(0,0,0),
        Vector v0 = Vector(0,0,0),
        Matrix orient0 = (IdentityMatrix<4,float>()));

    inline Ptr<FlightControls> getFlightControls() { return controls; }

    inline float getMaxThrust() { return max_thrust; }
    inline void setMaxThrust(float new_thrust) { max_thrust = new_thrust; }

    //IEngine
    virtual void setControls(Ptr<DataNode> controls);
    virtual void run();

    // IPositionProvider
    virtual Vector getLocation();
    virtual Vector getFrontVector();
    virtual Vector getRightVector();
    virtual Vector getUpVector();
    virtual void getOrientation(Vector * up, Vector * right, Vector * front);

    // IMovementProvider
    virtual Vector getMovementVector();

    // IPositionReceiver
    virtual void setLocation(const Vector &);
    virtual void setOrientation(const Vector & up,
                                const Vector & right,
                                const Vector & front);

    // IMovementReceiver
    virtual void setMovementVector(const Vector &);

protected:
    Ptr<IGame> thegame;
    Ptr<FlightControls> controls;
    Vector p,v;
    Matrix orient;
    float roll_speed, pitch_speed, yaw_speed;

    // static parameters
    float max_thrust, wing_area, mass;
};

#endif
