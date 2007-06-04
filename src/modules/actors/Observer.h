#ifndef OBSERVER_H
#define OBSERVER_H

#include "simpleactor.h"

class Observer : public SimpleActor, public SigObject
{
public:
    Observer(Ptr<IGame>);

    virtual void action();
    virtual void draw();
    
    virtual int getNumViews();
    virtual Ptr<IView> getView(int n);

protected:
    void mapEvents();
    void setDollying(bool);
    void setPanning(bool);
    
    inline bool isDollying() { return dollying; }
    inline bool isPanning() { return panning; }
    void stop();
    
    bool dollying, panning;
};


#endif

