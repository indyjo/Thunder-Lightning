#ifndef CARENGINE_H
#define CARENGINE_H

#include <landscape.h>
#include "engine.h"
#include "controls.h"
#include <interfaces/IGame.h>


struct CarParams {
    float mass, max_accel, max_brake, max_rot_per_meter;
    float drag_coeff, res_coeff;
    Vector tripod[3]; // Used for orientation determination on 3d terrain
                      // center back, left front, right front
    
    CarParams(
        float mass = 1000.0f,
        float max_accel  = 1000.0f * 1.85f,       // ~ 100 km/h in 15s
        float max_brake  = 1000.0f * 2.77f,       // ~ 100 km/h in 10s
        float min_curve_radius = 10.0f,            // 3m min curve radius
        float drag_coeff = 0.5f * 5.0f * 0.6f * 1.293f,
        float res_coeff = 58.185f)
    :   
        mass(mass),
        max_accel(max_accel),
        max_brake(max_brake),
        max_rot_per_meter(1.0f / min_curve_radius),
        drag_coeff(drag_coeff),
        res_coeff(res_coeff)
    {
        tripod[0] = Vector( 0.0f,  0.0f, -2.5f);
        tripod[1] = Vector(-1.0f,  0.0f,  2.5f);
        tripod[2] = Vector( 1.0f,  0.0f,  2.5f);
    }
};
        

class CarEngine: virtual public IEngine, protected CarParams {
public:
    CarEngine(Ptr<IGame> game,
              const CarParams & params = CarParams());

    // IEngine
    virtual void setControls(Ptr<Controls> controls);
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
    void update();

protected:
    Ptr<IGame> thegame;
    Ptr<ITerrain> terrain;
    Ptr<CarControls> controls;
    Vector p;
    float v;
    Vector front, right, up;
};


#endif
