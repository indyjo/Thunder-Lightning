#ifndef IMOVEMENTPROVIDER_H
#define IMOVEMENTPROVIDER_H

#include <interfaces/IPositionProvider.h>

struct IMovementProvider: virtual public IPositionProvider
{
    virtual Vector getMovementVector()=0;
};

#endif
