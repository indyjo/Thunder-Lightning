#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <SDL/SDL_Event.h>

class EventHandler {
    EventHandler *parent;
public:
    EventHandler(EventHandler *parent = 0) : parent(parent) { };
    virtual void handle(SDL_Event & ev);
    virtual void endOfStep(float step_time);
    
    static struct TheHandlers {
        SystemEventHandler      system_event_handler;
        DroneEventHandler       drone_event_handler;

        TheHandlers();
    } handlers;
};

#endif
