#ifndef RIGIDENGINE_H
#define RIGIDENGINE_H

#include <landscape.h>
#include <modules/math/Transform.h>
#include <modules/physics/RigidBody.h>
#include <interfaces/IGame.h>

#include "engine.h"


class RigidEngine: virtual public IEngine, public RigidBody
{
protected:
    Ptr<IGame> thegame;
public:
    RigidEngine(Ptr<IGame> game);

    //IEngine
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

    // Collidable helpers
    void integrate(float delta_t, Transform * transforms);
    void update(float delta_t, const Transform * new_transforms);
};



#endif
