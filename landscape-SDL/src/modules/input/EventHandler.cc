#include "EventHandler.h"
#include "SystemEventHandler.h"
#include "InputEventHandler.h"
#include "FlightEventHandler.h"
#include "DroneEventHandler.h"


void EventHandler::handle(SDL_Event & ev) {
    if (parent) parent->handle(ev);
}

void EventHandler::endOfStep(float step_time) {
    if (parent) parent->endOfStep(step_time);
}

