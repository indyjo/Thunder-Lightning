#ifndef TNL_CEGUI_EVENT_FILTER_H
#define TNL_CEGUI_EVENT_FILTER_H

#include <interfaces/IGame.h>
#include <remap.h>

class Clock;

class CEGUIEventFilter : public IEventFilter, public SigC::Object {
    bool pass_events;
    Ptr<Clock> clock;
public:
    CEGUIEventFilter(IGame & game, bool pass_events=true);
    virtual ~CEGUIEventFilter();
    
    virtual bool feedEvent(SDL_Event & ev);
private:
    void tick();
};

#endif

