#ifndef DEBUGOBJECT_H
#define DEBUGOBJECT_H

#include <landscape.h>
#include <modules/actors/simpleactor.h>

class DebugActor : public SimpleActor {
    float ttl;
public:
    DebugActor(Ptr<IGame> game, const Vector & p, const char * text, float ttl=3.0f);
    DebugActor(const Vector & p, const char * text, float ttl=3.0f);
    virtual void action();
};

#endif
