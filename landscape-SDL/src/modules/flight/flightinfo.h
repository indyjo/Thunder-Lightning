#ifndef FLIGHTINFO_H
#define FLIGHTINFO_H

#include <landscape.h>

class FlightInfo {
    Vector p,velocity,acceleration;
    float speed, accel, roll, d_roll, pitch, d_pitch;
    float height, d_height, altitude, d_altitude, course, d_course, aoa;
    bool collision_warning;
public:
    inline FlightInfo()
    : speed(0), accel(0), roll(0), d_roll(0), pitch(0), d_pitch(0),
      height(0), d_height(0),
      altitude(0), d_altitude(0), course(0), d_course(0),
      collision_warning(false), aoa(0), p(0,0,0),
      velocity(0,0,0), acceleration(0,0,0)
    { }
    inline const Vector & getCurrentLocation() const { return p; }
    inline float getCurrentSpeed()       const { return speed; }
    inline float getCurrentAccel()       const { return accel; }
    inline float getCurrentRoll()        const { return roll; }
    inline float getCurrentRollSpeed()   const { return d_roll; }
    inline float getCurrentPitch()       const { return pitch; }
    inline float getCurrentPitchSpeed()  const { return d_pitch; }
    inline float getCurrentHeight()      const { return height; }
    inline float getCurrentDHeight()     const { return d_height; }
    inline float getCurrentAltitude()    const { return altitude; }
    inline float getCurrentDAltitude()   const { return d_altitude; }
    inline float getCurrentCourse()      const { return course; }
    inline float getCurrentCourseSpeed() const { return d_course; }
    inline bool  collisionWarning()      const { return collision_warning; }
    inline float getCurrentAoA()         const { return aoa; }
    inline const Vector & getCurrentVelocity() const { return velocity; }
    
    void update(double delta_t, IMovementProvider & mp, ITerrain & ter);
    void dump();
};

    


#endif
