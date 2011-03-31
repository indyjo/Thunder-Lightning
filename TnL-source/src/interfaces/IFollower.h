#ifndef IFOLLOWER_H
#define IFOLLOWER_H

#include <object.h>

struct IActor;

struct IFollower : virtual public Object
{
public:
    virtual void follow(Ptr<IActor>)=0;
};

#endif
