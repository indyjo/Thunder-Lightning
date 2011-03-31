#ifndef COLLIDE_H
#define COLLIDE_H

#include "Vector.h"
#include "Line.h"
#include "Plane.h"

namespace Collide {

bool points(const Vector & p0, const Vector & p1, float r0=0, float r1=0);
bool movingPoints(
        const Line & l0, const Line & l1,
        float r0, float r1,
        float *t0, float *t1);
bool lineOnPlane(const Line & line, const Plane & plane, float * t);

}

#endif
