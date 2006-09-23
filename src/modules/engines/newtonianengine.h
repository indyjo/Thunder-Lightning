#ifndef NEWTONIANENGINE_H
#define NEWTONIANENGINE_H

#include <landscape.h>
#include <interfaces/IGame.h>
#include "engine.h"

class NewtonianEngine : public IEngine {
    Ptr<IGame> thegame;
    Vector p, v;
    Vector up, right, front;
    
public:
    NewtonianEngine(Ptr<IGame> game,
                    const Vector & p0 = Vector(0,0,0),
                    const Vector & v0 = Vector(0,0,0))
        : thegame(game), p(p0), v(v0)
    { update(); }
    
    //IEngine
    virtual void setControls(Ptr<Controls> controls);
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
    
private:
    void update();
};

#endif
