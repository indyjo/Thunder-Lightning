#ifndef IPOSITIONRECEIVER_H
#define IPOSITIONRECEIVER_H

#include <object.h>
#include <modules/math/Vector.h>

struct IPositionReceiver : virtual public Object
{
    virtual void setLocation(const Vector &) = 0;
    virtual void setOrientation(const Vector & up,
                                const Vector & right,
                                const Vector & front) = 0;
};

#endif
