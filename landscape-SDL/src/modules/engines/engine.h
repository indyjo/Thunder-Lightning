#ifndef ENGINE_H
#define ENGINE_H

#include <landscape.h>

class IEngine : public IMovementProvider,
                public IMovementReceiver
{
public:
    virtual void run() = 0;
};

#endif
