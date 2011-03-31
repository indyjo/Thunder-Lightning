#ifndef OBSERVERENGINE_H
#define OBSERVERENGINE_H

#include <tnl.h>
#include <interfaces/IGame.h>
#include <modules/clock/clock.h>
#include <modules/math/Transform.h>
#include "engine.h"

class ObserverEngine : public IEngine {
    Ptr<Clock> clock;
    Transform xform;
    Vector v;
    Ptr<DataNode> controls;
public:
    ObserverEngine(Ptr<IGame> game);
    
    //IEngine
    virtual void setControls(Ptr<DataNode> controls);
    virtual void run();
    
    // IPositionProvider
    virtual Vector getLocation();
    virtual Vector getFrontVector();
    virtual Vector getRightVector();
    virtual Vector getUpVector();
    virtual void getOrientation(Vector * up, Vector * right, Vector * front);

    // IMovementProvider
    virtual Vector getMovementVector();

    // IPositionReceiver
    virtual void setLocation(const Vector &);
    virtual void setOrientation(const Vector & up,
                                const Vector & right,
                                const Vector & front);

    // IMovementReceiver
    virtual void setMovementVector(const Vector &);
};

#endif
