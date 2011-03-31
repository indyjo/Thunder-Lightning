#ifndef IPOSITIONPROVIDER_H
#define IPOSITIONPROVIDER_H

#include <object.h>
#include <modules/math/Vector.h>

struct IPositionProvider : virtual public Object
{
    virtual Vector getLocation() = 0;
    virtual Vector getFrontVector() = 0;
    virtual Vector getRightVector() = 0;
    virtual Vector getUpVector() = 0;
    virtual void getOrientation(  Vector * up,
                                    Vector * right,
                                    Vector * front) = 0;
};


#endif
