#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include <landscape.h>
#include "flightinfo.h"
#include <modules/engines/controls.h>

#define AP_SPEED        0
#define AP_ACCEL        1
#define AP_ROLL         2
#define AP_PITCH        3
#define AP_HEIGHT       4
#define AP_ALTITUDE     5
#define AP_COURSE       6

#define AP_SPEED_MASK       (2<<AP_SPEED)
#define AP_ACCEL_MASK       (2<<AP_ACCEL)
#define AP_ROLL_MASK        (2<<AP_ROLL)
#define AP_PITCH_MASK       (2<<AP_PITCH)
#define AP_HEIGHT_MASK      (2<<AP_HEIGHT)
#define AP_ALTITUDE_MASK    (2<<AP_ALTITUDE)
#define AP_COURSE_MASK      (2<<AP_COURSE)

class AutoPilot : virtual public SigObject {
    float speed_target;
    float accel_target;
    float roll_target;
    float pitch_target;
    float height_target;
    float altitude_target;
    float course_target;
    
    int mode;
    
public:
    AutoPilot() : mode(0) { }
    
    void reset() { mode = 0; }
    
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
    
    void fly( const FlightInfo & fi, FlightControls & ctrls );

protected:
    int implies(int module);
    void handleSpeed( const FlightInfo & fi, FlightControls & ctrls );
    void handleAccel( const FlightInfo & fi, FlightControls & ctrls );
    void handleRoll( const FlightInfo & fi, FlightControls & ctrls );
    void handlePitch( const FlightInfo & fi, FlightControls & ctrls );
    void handleHeight( const FlightInfo & fi, FlightControls & ctrls );
    void handleAltitude( const FlightInfo & fi, FlightControls & ctrls );
    void handleCourse( const FlightInfo & fi, FlightControls & ctrls );
};

#endif
