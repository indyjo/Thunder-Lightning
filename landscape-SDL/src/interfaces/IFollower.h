#ifndef IFOLLOWER_H
#define IFOLLOWER_H

#include <object.h>

class IActor;

class IFollower : virtual public Object
{
public:
    virtual void follow(Ptr<IActor>)=0;
};

#endif
