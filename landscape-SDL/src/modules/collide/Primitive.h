#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <landscape.h>
#include <modules/collide/BoundingBox.h>
#include <modules/math/Interval.h>
#include <modules/math/Transform.h>

namespace Collide {

typedef XTransform<Interval> ITransform;
typedef XVector<3,Interval> IVector;

union Hints {
    struct {
        float exactness;
        float max_box_dim;
    } box;
    struct {
        float exactness;
        bool on_edge;
        int edge;
        bool must_divide_time;
    } triangle_sphere;
    struct TriTri {
        float exactness;
        enum { VERTEX_TRIANGLE, TRIANGLE_VERTEX, EDGE_EDGE } type;
        int a,b;
        bool must_divide_time;
    } triangle_triangle;
};


bool intersectBoxBox(const BoundingBox & box1,
                     const IVector & pos1, const IVector * orient1,
                     const BoundingBox & box2,
                     const IVector & pos2, const IVector * orient2,
                     Hints & hints);

bool intersectBoxSphere(const BoundingBox & box,
                        const IVector & pos1, const IVector * orient1,
                        float radius, const IVector & pos2,
                        Hints & hints);

bool intersectTriangleSphere(const Vector * triangle,
                             const ITransform & T1,
                             float radius, const IVector & pos2,
                             Hints & hints);

bool intersectTriangleTriangle(const Vector * triangle1,
                               const ITransform & T1,
                               const Vector * triangle2,
                               const ITransform & T2,
                               Hints & hints);

} // namespace Collide

#endif
