#ifndef COLLIDE_GEOMETRYINSTANCE_H
#define COLLIDE_GEOMETRYINSTANCE_H

#include <modules/math/Transform.h>

#include "Collidable.h"

namespace Collide {

struct GeometryInstance {
    Ptr<Collidable> collidable;
    Transform * transforms_0; // The state at the beginning and the end of the
    Transform * transforms_1; // active time interval

    GeometryInstance();
    GeometryInstance(Ptr<Collidable> c);
    GeometryInstance(const GeometryInstance & g);

    ~GeometryInstance();

    GeometryInstance & operator=(const GeometryInstance & g);
};

} // namespace Collide

#endif
