#ifndef ENGINE_H
#define ENGINE_H

#include <landscape.h>
#include <DataNode.h>
#include <interfaces/IMovementProvider.h>
#include <interfaces/IMovementReceiver.h>


struct IEngine : public IMovementProvider,
                 public IMovementReceiver
{
    virtual void setControls(Ptr<DataNode> controls) = 0;
    virtual void run() = 0;
};

#endif
