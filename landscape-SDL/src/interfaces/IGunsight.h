#ifndef IGUNSIGHT_H
#define IGUNSIGHT_H

#include <interfaces/IDrawable.h>

class IActor;

class IGunsight: public IDrawable
{
public:
    virtual void enable()=0;
    virtual void disable()=0;
    virtual Ptr<IActor> getCurrentTarget()=0;
};

#endif
