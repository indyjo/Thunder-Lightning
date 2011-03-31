#ifndef IEFFECTOR_H
#define IEFFECTOR_H

#include <object.h>
#include <modules/physics/RigidBody.h>

class DataNode;

struct IEffector : public Object {
    virtual void applyEffect(RigidBody &rigid, Ptr<DataNode> controls)=0;
};

#endif

