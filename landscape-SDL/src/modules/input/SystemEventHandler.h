#ifndef SYSTEMEVENTHANDLER_H
#define SYSTEMEVENTHANDLER_H

#include <landscape.h>
#include "EventHandler.h"

class SystemEventHandler : public EventHandler {
public:
    ActionSignal endgame, faster, slower, toggle_debug;

    SystemEventHandler(EventHandler *parent = 0) : parent(parent) { };
    virtual void handle(SDL_Event & ev);
};


#endif