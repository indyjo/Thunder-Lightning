#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <tnl.h>
#include <modules/collide/BoundingBox.h>
#include <modules/math/Interval.h>
#include <modules/math/Transform.h>

namespace Collide {

typedef XTransform<Interval> ITransform;
typedef XVector<3,Interval> IVector;

class BoundingNode;
struct GeometryInstance;

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

bool intersectTriangleSphere(const IVector * tri,
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

bool isPointInPrism(const Vector &p3d, const Vector *tri, const Vector & normal);

bool intersectLineTriangle(const Vector &a, const Vector &b,
                           Vector * tri_lcs,
                           const Transform & xform,
                           Vector *out_x=0, Vector *out_normal=0);

bool earliestIntersectionLineTriangle(bool previous_result,
                                      const Vector &a, const Vector &b,
                                      Vector * tri_lcs,
                                      const Transform & xform,
                                      Vector *inout_x, Vector *out_normal=0);

bool intersectLineBox(bool solid_box,
                      const Vector &a, const Vector &b,
                      const BoundingBox & box,
                      const Transform & xform,
                      Vector *out_x=0, Vector *out_normal=0);
                               
bool intersectLineNode(const Vector &a, const Vector &b,
                       int xform_id,
                       const Transform & xform,
                       const GeometryInstance * geom_instance,
                       const BoundingNode * node,
                       Vector * out_x=0, Vector *out_normal=0);

} // namespace Collide

#endif
