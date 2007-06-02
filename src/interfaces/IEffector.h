#ifndef IEFFECTOR_H
#define IEFFECTOR_H

#include <object.h>
#include <modules/physics/RigidBody.h>

struct IEffector : public Object {
    virtual void applyEffect(RigidBody &rigid)=0;
};

#endif

