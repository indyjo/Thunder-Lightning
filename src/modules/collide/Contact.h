#ifndef COLLIDE_CONTACT_H
#define COLLIDE_CONTACT_H

#include <object.h>
#include <modules/math/Vector.h>

namespace Collide {

class Collidable;

struct Contact {
    Ptr<Collidable> collidables[2];
    int domains[2];
    // position and normal of collision.
    // The normal is pointing towards collidables[0]
    Vector p, n;
    // velocity of the objects wrt normal.
    // v[0] should be negative, v[1] positive
    float v[2];

    void applyCollisionImpulse();
};

} //namespace Collide

#endif
