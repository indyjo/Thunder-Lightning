#ifndef ITERRAIN_H
#define ITERRAIN_H

#include <modules/math/Vector.h>
#include <interfaces/IDrawable.h>

class ITerrain: public IDrawable
{
public:
    virtual float getHeightAt(float x, float z)=0;
    // lineCollides: test if line between a and b intersects terrain
    // Stores intersection point in x
    virtual bool lineCollides(Vector a, Vector b, Vector * x)=0;
};

#endif
