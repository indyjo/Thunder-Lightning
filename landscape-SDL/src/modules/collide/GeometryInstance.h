#ifndef COLLIDE_GEOMETRYINSTANCE_H
#define COLLIDE_GEOMETRYINSTANCE_H

#include <modules/math/Transform.h>

#include "Collidable.h"

namespace Collide {

struct GeometryInstance {
    Ptr<Collidable> collidable;
    Transform * transforms_0; // The state at the beginning and the end of the
    Transform * transforms_1; // active time interval

    inline GeometryInstance()
    {
        collidable = 0;
        transforms_0 = new Transform[0];
        transforms_1 = new Transform[0];
    }

    inline GeometryInstance(Ptr<Collidable> c)
    :   collidable(c)
    {
        int n = c->getBoundingGeometry()->getNumOfTransforms();
        transforms_0 = new Transform[n];
        transforms_1 = new Transform[n];
    }

    inline GeometryInstance(const GeometryInstance & g)
    :   collidable(g.collidable)
    {
        int n;
        if(collidable) {
            n = collidable->getBoundingGeometry()->getNumOfTransforms();
        } else n = 0;
        transforms_0 = new Transform[n];
        transforms_1 = new Transform[n];
        for(int i=0; i<n; i++) {
            transforms_0[i] = g.transforms_0[i];
            transforms_1[i] = g.transforms_1[i];
        }
    }

    inline ~GeometryInstance() {
        delete [] transforms_0;
        delete [] transforms_1;
    }

    inline GeometryInstance & operator=(const GeometryInstance & g)
    {
        delete [] transforms_0;
        delete [] transforms_1;

        collidable = g.collidable;
        int n;
        if(collidable) {
            n = collidable->getBoundingGeometry()->getNumOfTransforms();
        } else n = 0;
        transforms_0 = new Transform[n];
        transforms_1 = new Transform[n];
        for(int i=0; i<n; i++) {
            transforms_0[i] = g.transforms_0[i];
            transforms_1[i] = g.transforms_1[i];
        }
        
        return *this;
    }
};

} // namespace Collide

#endif
