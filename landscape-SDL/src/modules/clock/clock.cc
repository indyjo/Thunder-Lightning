#include <SDL.h>
#include "clock.h"

#define TICK_SECS 0.001

Clock::Clock()
: time_factor(1),
  frame_delta(0), real_frame_delta(0),
  step_delta(0), real_step_delta(0),
  time_left(0), pause_mode(false)
{
    ticks = measureTicks();
}

void Clock::setTimeFactor(double tf) {
    double ratio = tf / time_factor;
    time_factor = tf;
    frame_delta *= ratio;
    step_delta *= ratio;
    time_left *= ratio;
}

void Clock::update() {
    int new_ticks = measureTicks();
    real_frame_delta = (double)(new_ticks - ticks) * TICK_SECS;
    if (!pause_mode) {
        frame_delta = time_factor * real_frame_delta;
        time_left += frame_delta;
    }
    ticks = new_ticks;
}

bool Clock::catchup(double time) {
    if (pause_mode) return false;
    if (time_left == 0.0) return false;
    if (time_left < time) time = time_left;
    time_left -= time;
    step_delta = time;
    real_step_delta = time / time_factor;
    return true;
}

int Clock::measureTicks() {
    return SDL_GetTicks();
}
