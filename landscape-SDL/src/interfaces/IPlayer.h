#ifndef IPLAYER_H
#define IPLAYER_H

#include <interfaces/IActor.h>

class IPlayer: virtual public IActor
{
public:
    virtual void setThrottle(float)=0;
    virtual float getThrottle()=0;
};

#endif
