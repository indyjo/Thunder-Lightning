#ifndef IMOVEMENT_RECEIVER_H
#define IMOVEMENT_RECEIVER_H

#include <interfaces/IPositionReceiver.h>

struct IMovementReceiver: virtual public IPositionReceiver
{
    virtual void setMovementVector(const Vector &)=0;
};

#endif
