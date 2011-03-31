#include "Collide.h"
#include "Solve.h"

namespace Collide {

bool points(const Vector & p0, const Vector & p1, float r0, float r1) {
    float r = r0+r1;
    return (p1-p0).lengthSquare() < r*r;
}

bool movingPoints(
        const Line & l0, const Line & l1,
        float r0, float r1,
        float *t0, float *t1) {
    float r = r0 + r1;
    Vector dp = l1.P() - l0.P();
    Vector dv = l1.V() - l0.V();
    
    float a = dv * dv;
    float b = 2.0 * (dp * dv);
    float c = dp * dp - r*r;
    
    return Solve::order2(a,b,c,t0,t1);
}

bool lineOnPlane(const Line & line, const Plane & plane, float * t) {
    if (line.V()*plane.N() == 0) return false;
    
    *t = ( -line.P()*plane.N() - plane.A()) / (line.V()*plane.N());
    return true;
}

} // namespace Collide
