#ifndef CLOCK_H
#define CLOCK_H

#include <tnl.h>

class Clock : public Object {
    int ticks;
    double time_factor;
    double frame_delta, real_frame_delta;
    double step_delta, real_step_delta;
    double time_left;
    bool pause_mode, initialized;
    
public:
    Clock();
        
    // Time passed since last frame (in seconds)
    inline double getFrameDelta() { return frame_delta; }
    // Time passed since last game step (in seconds)
    inline double getStepDelta()  { return step_delta; }
    
    // Real time passed since last frame
    inline double getRealFrameDelta() { return real_frame_delta; }
    // Real time passed since last game step
    inline double getRealStepDelta() { return real_step_delta; }
    
    inline double getTimeFactor() { return time_factor; }
    void setTimeFactor(double);
    
    // Called each frame to get the new time
    void update();
    // Called for each step of the given length.
    // Returns false when time is caught up.
    bool catchup(double time);
    // Catches up all available time left without setting delta values
    void skip();
    
    // Start and stop pause
    inline void pause() { pause_mode = true; }
    inline void resume() { pause_mode = false; }
    inline bool isPaused() { return pause_mode; }
    
protected:
    int measureTicks();
};

#endif
