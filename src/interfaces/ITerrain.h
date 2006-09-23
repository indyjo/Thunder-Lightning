#ifndef ITERRAIN_H
#define ITERRAIN_H

#include <modules/math/Vector.h>
#include <interfaces/IDrawable.h>

struct ITerrain: public IDrawable
{
public:
    // queries for terrain height at point (x,z)
    // optionally returns normal
    virtual float getHeightAt(float x, float z, Vector *out_normal=0)=0;
    // lineCollides: test if line between a and b intersects terrain
    // Stores intersection point in x
    // optionally returns normal at x
    virtual bool lineCollides(Vector a, Vector b, Vector * x, Vector *out_normal=0)=0;
};

#endif
