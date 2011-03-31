#ifndef RIGIDENGINE_H
#define RIGIDENGINE_H

#include <vector>
#include <tnl.h>
#include <modules/math/Transform.h>
#include <modules/physics/RigidBody.h>
#include <interfaces/IGame.h>
#include <interfaces/IEffector.h>
#include <DataNode.h>

#include "engine.h"

class RigidEngine: virtual public IEngine, public RigidBody
{
protected:
    Ptr<IGame> thegame;
    
    typedef std::vector<Ptr<IEffector> > Effectors;
    Effectors effectors;
    Ptr<DataNode> controls;
public:
    RigidEngine(Ptr<IGame> game);
    
    //IEngine
    inline Ptr<DataNode> getControls() { return controls; }
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

    // Collidable helpers
    void integrate(float delta_t, Transform * transforms);
    void update(float delta_t, const Transform * new_transforms);
    
    // Effectors management
    void addEffector(Ptr<IEffector> effector);
    void removeEffector(Ptr<IEffector> effector);
    
private:
    void clearAndApplyEffectors();
};


#endif
