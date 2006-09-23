#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include <landscape.h>
#include "flightinfo.h"
#include <modules/engines/controls.h>
#include <modules/math/PIDController.h>

#define AP_SPEED        0
#define AP_ACCEL        1
#define AP_ROLL         2
#define AP_PITCH        3
#define AP_HEIGHT       4
#define AP_ALTITUDE     5
#define AP_COURSE       6
#define AP_DIRECTION    7
#define AP_VECTOR       8

#define AP_SPEED_MASK       (2<<AP_SPEED)
#define AP_ACCEL_MASK       (2<<AP_ACCEL)
#define AP_ROLL_MASK        (2<<AP_ROLL)
#define AP_PITCH_MASK       (2<<AP_PITCH)
#define AP_HEIGHT_MASK      (2<<AP_HEIGHT)
#define AP_ALTITUDE_MASK    (2<<AP_ALTITUDE)
#define AP_COURSE_MASK      (2<<AP_COURSE)
#define AP_DIRECTION_MASK   (2<<AP_DIRECTION)
#define AP_VECTOR_MASK      (2<<AP_VECTOR)

class AutoPilot : virtual public SigObject {
    float speed_target;
    float accel_target;
    float roll_target;
    PIDController<float> roll_controller;
    float pitch_target;
    PIDController<float> pitch_controller;
    float height_target;
    float altitude_target;
    float course_target;
    PIDController<float> course_controller;
    Vector direction_target, d_direction_target;
    Vector vector_target, d_vector_target;
    
    int mode;
    
public:
    AutoPilot();
    
    void reset();
    
    int  getMode() { return mode; }
    bool isEnabled(int module) { return mode & (2<<module); }
    
    // return: true for success, false for failure
    bool setMode(int mode);
    bool enable(int module);

    void setTargetSpeed(float target) { speed_target = target; }
    float getTargetSpeed() { return speed_target; }
    
    void setTargetAccel(float target) { accel_target = target; }
    float getTargetAccel() { return accel_target; }
    
    void setTargetRoll(float target) { roll_target = target; }
    float getTargetRoll() { return roll_target; }

    void setTargetPitch(float target) { pitch_target = target; }
    float getTargetPitch() { return pitch_target; }
    
    void setTargetHeight(float target) { height_target = target; }
    float getTargetHeight() { return height_target; }
    
    void setTargetAltitude(float target) { altitude_target = target; }
    float getTargetAltitude() { return altitude_target; }
    
    void setTargetCourse(float target) { course_target = target; }
    float getTargetCourse() { return course_target; }
    
    void setTargetDirection(const Vector & dir,
                            const Vector & deriv=Vector(0,0,0))
    {
    	direction_target = dir;
    	d_direction_target = deriv;
    }
    Vector getTargetDirection() { return direction_target; }
    
    void setTargetVector(const Vector & vec,
                         const Vector & deriv=Vector(0,0,0))
    {
    	vector_target = vec;
    	d_vector_target = deriv;
    }
    Vector getTargetVector() { return vector_target; }
    
    void fly( float dt, const FlightInfo & fi, FlightControls & ctrls );

protected:
    int implies(int module);
    void handleSpeed( const FlightInfo & fi, FlightControls & ctrls );
    void handleAccel( const FlightInfo & fi, FlightControls & ctrls );
    void handleRoll( float dt, const FlightInfo & fi, FlightControls & ctrls );
    void handlePitch( float dt, const FlightInfo & fi, FlightControls & ctrls );
    void handleHeight( const FlightInfo & fi, FlightControls & ctrls );
    void handleAltitude( const FlightInfo & fi, FlightControls & ctrls );
    void handleCourse( float dt, const FlightInfo & fi, FlightControls & ctrls );
    void handleDirection( const FlightInfo & fi, FlightControls & ctrls );
    void handleVector( const FlightInfo & fi, FlightControls & ctrls );
};

#endif
