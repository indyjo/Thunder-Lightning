#ifndef IMOVEMENTPROVIDER_H
#define IMOVEMENTPROVIDER_H

#include <interfaces/IPositionProvider.h>

class IMovementProvider: virtual public IPositionProvider
{
public:
    virtual Vector getMovementVector()=0;
};

#endif
