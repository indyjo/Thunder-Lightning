#ifndef CHASINGENGINE_H
#define CHASINGENGINE_H

#include <tnl.h>
#include <interfaces/IGame.h>
#include <modules/clock/clock.h>
#include <modules/math/Transform.h>
#include "engine.h"

class ChasingEngine : public IEngine {
    Ptr<Clock> clock;
    Ptr<IPositionProvider> target;
    Transform current_xform, previous_xform, pre_xform, post_xform, effective_xform;
    Vector v;
    float pos_thalf, rot_thalf;
public:
    ChasingEngine(
        Ptr<IGame> game,
        Ptr<IPositionProvider> target,
        float pos_thalf=0,
        float rot_thalf=0,
        Transform pre_xform=Transform::identity(),
        Transform post_xform=Transform::identity());
    
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

protected:
    Transform getTargetTransform();
};

#endif
