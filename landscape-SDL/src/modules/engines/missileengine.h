#ifndef MISSILEENGINE_H
#define MISSILEENGINE_H

#include <landscape.h>
#include "engine.h"
#include "rigidengine.h"

class MissileEngine : public IEngine {
protected:
    Ptr<IGame> thegame;
    Vector p, v, d;
    float thrust;
    float mass;
    float frontal_area, side_area;
    Vector up, right, front;
    
public:
    MissileEngine(Ptr<IGame> game,
                    const Vector & p0 = Vector(0,0,0),
                    const Vector & v0 = Vector(0,0,0),
                    const Vector & d0 = Vector(0,0,0),
                    float thrust0 = 74000.0,    // N
                    float mass0 = 200.0,        // kg
                    float frontal_area0 = .018, // m^2
                    float side_area0 =    .63   // m^2
        )
        : thegame(game), p(p0), v(v0), d(d0),
          thrust(thrust0), mass(mass0),
          frontal_area(frontal_area0), side_area(side_area0)
    { update(); }
    
    inline Vector getDirection() { return d; } // same as getFrontVector
    inline void setDirection(const Vector & new_d) { d = new_d; update(); }
    
    inline float getThrust() { return thrust; }
    inline void setThrust(float new_thrust) { thrust = new_thrust; }
    
    inline float getMass() { return mass; }
    inline void setMass(float new_mass) { mass = new_mass; }
    
    inline float getFrontalArea() { return frontal_area; }
    inline void setFrontalArea(float new_area) { frontal_area = new_area; }
    inline float getSideArea() { return side_area; }
    inline void setSideArea(float new_area) { side_area = new_area; }
    
    //IEngine
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
    
private:
    void update();
};


class SmartMissileEngine : public MissileEngine {
    float max_angular_speed;
    Vector target;
    Vector error;
    Vector omega;
public:
    SmartMissileEngine( Ptr<IGame> game,
                    const Vector & p0 = Vector(0,0,0),
                    const Vector & v0 = Vector(0,0,0),
                    const Vector & d0 = Vector(0,0,0),
                    float thrust0 = 30000.0,    // N
                    float mass0 = 200.0,        // kg
                    float frontal_area0 = .024, // m^2
                    float side_area0 =    .63)  // m^2
        : MissileEngine(game, p0, v0, d0, thrust0, mass0,
            frontal_area0, side_area0),
          max_angular_speed( 3.14 ),           // ~180°/s
          target(0,0,0),
          error(0,0,0),
          omega(0,0,0)
    { }
    
    inline float getMaxAngularSpeed() { return max_angular_speed; }
    inline void setMaxAngularSpeed( float new_v ) { max_angular_speed = new_v; }
    
    inline const Vector & getTarget() { return target; }
    inline void setTarget(const Vector & new_t) { target = new_t; }
    
    void run();
};


class MissileEngine2 : public RigidEngine {
    float front_area, side_area;
public:
    inline MissileEngine2(Ptr<IGame> game,
        float front_area, float side_area)
    :   RigidEngine(game),
        front_area(front_area),
        side_area(side_area)
    { }

    virtual void run();
};

#endif
