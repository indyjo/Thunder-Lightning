#ifndef IMOVEMENT_RECEIVER_H
#define IMOVEMENT_RECEIVER_H

#include <interfaces/IPositionReceiver.h>

class IMovementReceiver: virtual public IPositionReceiver
{
public:
    virtual void setMovementVector(const Vector &)=0;
};

#endif
