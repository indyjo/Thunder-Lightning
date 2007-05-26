#include "Primitive.h"
#include <stdexcept>

namespace {

inline float exactness(float x) { return 0.0f; }
inline float exactness(const Interval & x) { return x.length(); }
template<class T, int N>
inline float exactness(const XVector<N,T> & v) {
    float max_e = 0.0f;
    for(int i=0; i<N; i++) {
        float e = exactness(v[i]);
        if (e > max_e) max_e = e;
    }
    return max_e;
}

} // namespace


namespace Collide {

bool intersectBoxBox(const BoundingBox & box1,
                     const IVector & pos1, const IVector * orient1,
                     const BoundingBox & box2,
                     const IVector & pos2, const IVector * orient2,
                     Hints & hints)
{
    IVector delta_x = pos2 - pos1;
    Interval A, B;
    for(int i=0; i<6; i++) {
        const IVector & axis = i<3 ? orient1[i] : orient2[i-3];
        A = std::abs(delta_x * axis);
        B = 0;
        for(int j=0; j<3; j++) {
            B += Interval(box1.dim[j]) * std::abs(axis * orient1[j]);
            B += Interval(box2.dim[j]) * std::abs(axis * orient2[j]);
        }
        if (A > B) return false;
    }

    hints.box.exactness = exactness(pos1+pos2);
    for(int i=0; i<3; i++) {
        if (i==0 || box1.dim[i] > hints.box.max_box_dim)
            hints.box.max_box_dim = box1.dim[i];
        if (box2.dim[i] > hints.box.max_box_dim)
            hints.box.max_box_dim = box2.dim[i];
    }
    return true;
}

bool intersectBoxSphere(const BoundingBox & box,
                        const IVector & pos1, const IVector * orient1,
                        float radius, const IVector & pos2,
                        Hints & hints)
{
    IVector delta_x = pos2 - pos1;
    Interval A, B;

    for (int i=0; i<3; i++) {
        const IVector & axis = orient1[i];
        A = std::abs(delta_x * axis);
        B = box.dim[i] + radius;

        //B=0;
        //for(int j=0; j<3; j++)
        //    B += box.dim[j] * std::abs(axis * orient1[j]);
        if (A > B)
            return false;
    }
    hints.box.exactness = exactness(pos1+pos2);
    for(int i=0; i<3; i++) {
        if (i==0 || box.dim[i] > hints.box.max_box_dim)
            hints.box.max_box_dim = box.dim[i];
    }
    return true;
}

bool intersectTriangleSphere(const IVector * tri,
                             float radius, const IVector & pos2,
                             Hints & hints)
{
    hints.triangle_sphere.must_divide_time = false;
    
    IVector normal = (tri[1]-tri[0]) % (tri[2]-tri[0]);
    try {
        Interval length = normal.lengthSquare();
        if (length < 1e-5f) return false; // degenerate triangle!
        length = sqrt(length);
        normal /= length; // normalize the triangle's normal;
    } catch(std::exception & e) {
        hints.triangle_sphere.must_divide_time = true;
        return true;
    }

    Interval dist = normal * (pos2 - tri[0]);
    if ( std::abs(dist) > radius ) return false;
    if ( dist.b > radius) dist.b = radius;
    if ( dist.a < -radius) dist.a = -radius;
    Interval r2 = square(radius) - square(dist*dist);
    Interval r;
    try {
        r = sqrt(square(radius) - square(dist*dist));
    } catch(std::exception & e) {
        hints.triangle_sphere.must_divide_time = true;
        return true;
    }

    hints.triangle_sphere.on_edge = false;
    for (int i=0; i<3; i++) {
        IVector edge_normal = normal % (tri[(i+1)%3]-tri[i]);
        try {
            edge_normal.normalize();
        } catch(std::exception & e) {
            hints.triangle_sphere.must_divide_time = true;
            return true;
        }

        Interval d = edge_normal * (pos2 - tri[i]);

        if (d < -r) return false;
        if (d < +r) {
            hints.triangle_sphere.on_edge = true;
            hints.triangle_sphere.edge = i;
            hints.triangle_sphere.exactness = exactness(pos2+tri[0]+tri[1]+tri[2]);
            return true;
        }
    }
    hints.triangle_sphere.exactness = exactness(pos2 - dist * normal);
    return true;
}

bool intersectTriangleSphere(const Vector * triangle,
                             const ITransform & T1,
                             float radius, const IVector & pos2,
                             Hints & hints)
{
    // transformed triangle coords
    IVector tri[3];
    /*
    ls_message("Performing triangle sphere test for triangle:\n");
    for(int i=0; i<3; ++i) triangle[i].dump();*/
    for(int i=0; i<3; i++) tri[i] = T1((IVector) triangle[i]);
    return intersectTriangleSphere(tri, radius, pos2, hints);
}

namespace {
void dump_ivector(const IVector &v) {
    ls_message("[(%f %f %f) (%f %f %f)]\n", v[0].a,v[1].a,v[2].a,v[0].b,v[1].b,v[2].b);
}

bool contains_zero(const IVector & v) {
    return v[0].a <= 0 && 0 <= v[0].b &&
        v[1].a <= 0 && 0 <= v[1].b &&
        v[2].a <= 0 && 0 <= v[2].b;
}

bool contains_zero(const Interval & x) {
    return x.a <= 0 && 0 <= x.b;
}

}

bool intersectTriangleVertex(const IVector * tri,
                             const IVector & normal,
                             const IVector * edge_normals,
                             const IVector & v,
                             Hints & hints)
{
    //first check for every dimension if v and tri could intersect
    for(int i=0; i<3; i++) {
        Interval triangle_interval = tri[0][i];
        for(int j=1; j<3; j++) {
            triangle_interval.a = std::min(triangle_interval.a, tri[j][i].a);
            triangle_interval.b = std::max(triangle_interval.b, tri[j][i].b);
        }
        //ls_message("testing {%f %f] against [%f %f]\n",
        //    v[i].a,v[i].b, triangle_interval.a, triangle_interval.b);
        if (!intersect(v[i], triangle_interval)) return false;
    }

    Interval d = (tri[0]-v) * normal;
    if (contains_zero(d)) return false;

    /*
    for(int i=0; i<3; i++) {
        IVector edge_normal = normal % (tri[(i+1)%3]-tri[i]);
        if (edge_normal*(v-tri[i]) < 0) return false;
    }
    */
    /*
    for(int i=0; i<3; i++) {
        IVector edge_normal = normal % (tri[(i+1)%3]-tri[i]);
        if (edge_normal*(v-tri[i]) < 0) return false;
    }
    */
    for(int i=0; i<3; i++) {
        if (edge_normals[i] * (v-tri[i]) < 0) return false;
    }

    hints.triangle_triangle.exactness = exactness(v /*+ tri[0]+tri[1]+tri[2]*/);
    /*
    ls_message("exactness=%f\n",hints.triangle_triangle.exactness);
    ls_message("d=[%f, %f]\n", d.a, d.b);
    ls_message("normal=\n");
    ls_message("(%+1.5f, %+1.5f)\n", normal[0].a, normal[0].b);
    ls_message("(%+1.5f, %+1.5f)\n", normal[1].a, normal[1].b);
    ls_message("(%+1.5f, %+1.5f)\n", normal[2].a, normal[2].b);
    */
    /*
    ls_message("Intersecting vertex/triangle:\n");
    ls_message("vector:      "); dump_ivector(v);
    ls_message("triangle 0:  "); dump_ivector(tri[0]);
    ls_message("triangle 1:  "); dump_ivector(tri[1]);
    ls_message("triangle 2:  "); dump_ivector(tri[2]);
    ls_message("normal:      "); dump_ivector(normal);
    ls_message("plane dist:  [%f %f]\n", d.a, d.b);
    ls_message("exactness:    %f\n", hints.triangle_triangle.exactness);
    */

    return true;
}

bool intersectEdgeEdge(const IVector & a, const IVector & b,
                       const IVector & c, const IVector & d,
                       Hints & hints)
{
    IVector normal = (b-a) % (d-c);
    Interval dist = (c-a) * normal;
    if (!contains_zero(dist)) return false;

    IVector edge_normal = normal % (b-a);
    Interval x = edge_normal * (c-a);
    Interval y = edge_normal * (d-a);
    if (x < 0 && y < 0 || x > 0 && y > 0)
        return false;

    edge_normal = normal % (d-c);
    x = edge_normal * (a-c);
    y = edge_normal * (b-c);
    if (x < 0 && y < 0 || x > 0 && y > 0)
        return false;

    hints.triangle_triangle.exactness = exactness(a+b+c+d);
    
    /*
    ls_message("Intersecting edge/edge:\n");
    ls_message("edge 1 a:    "); dump_ivector(a);
    ls_message("edge 1 b:    "); dump_ivector(b);
    ls_message("edge 2 c:    "); dump_ivector(c);
    ls_message("edge 2 d:    "); dump_ivector(d);
    ls_message("normal:      "); dump_ivector(normal);
    ls_message("plane dist:  [%f %f]\n", dist.a, dist.b);
    ls_message("exactness:    %f\n", hints.triangle_triangle.exactness);
    */

    return true;
}



bool intersectTriangleTriangle(const Vector * triangle1,
                               const ITransform & T1,
                               const Vector * triangle2,
                               const ITransform & T2,
                               Hints & hints)
{
    IVector tri1[3], tri2[3];
    for(int i=0; i<3; i++) {
        tri1[i] = T1(triangle1[i]);
        tri2[i] = T2(triangle2[i]);
    }

    // Check if any of the three main axes is a separating axis.
    // If yes, there can't be an intersection
    for(int i=0; i<3; i++) {
        Interval t1(tri1[0][i]);
        Interval t2(tri2[0][i]);
        for(int j=1; j<3; j++) {
            t1.a = std::min(t1.a, tri1[j][i].a);
            t1.b = std::max(t1.b, tri1[j][i].b);
            t2.a = std::min(t2.a, tri2[j][i].a);
            t2.b = std::max(t2.b, tri2[j][i].b);
        }
        if (!intersect(t1, t2)) return false;
    }


    IVector i_normal1, i_normal2;
    IVector i_edge_normals1[3], i_edge_normals2[3];

    // Compute normals and inward pointing edge normals
    {
        Vector  normal1, normal2;
        normal1 = (triangle1[1]-triangle1[0]) % (triangle1[2]-triangle1[0]);
        normal2 = (triangle2[1]-triangle2[0]) % (triangle2[2]-triangle2[0]);
        i_normal1 = T1.quat().rot(normal1);
        i_normal2 = T2.quat().rot(normal2);

        for(int i=0; i<3; i++) {
            Vector edge_normal = normal1 % (triangle1[(i+1)%3]-triangle1[i]);
            i_edge_normals1[i] = T1.quat().rot(edge_normal);
            edge_normal = normal2 % (triangle2[(i+1)%3]-triangle2[i]);
            i_edge_normals2[i] = T2.quat().rot(edge_normal);
        }
    }

    // check if any of the normals contain the zero vector.
    // If it does, we have to subdivide time
    if (contains_zero(i_normal1) || contains_zero(i_normal2)) {
        hints.triangle_triangle.must_divide_time = true;
        return true;
    }

    hints.triangle_triangle.must_divide_time = false;
    try {
        for(int i=0; i<3; i++) {
            if (intersectTriangleVertex(tri2, i_normal2, i_edge_normals2,
                                        tri1[i], hints)) {
                hints.triangle_triangle.type = Hints::TriTri::VERTEX_TRIANGLE;
                hints.triangle_triangle.a = i;
                return true;
            }
            if (intersectTriangleVertex(tri1, i_normal1, i_edge_normals1,
                                        tri2[i], hints)) {
                hints.triangle_triangle.type = Hints::TriTri::TRIANGLE_VERTEX;
                hints.triangle_triangle.b = i;
                return true;
            }
        }
        for(int i=0; i<3; i++) for (int j=i; j<3; j++) {
            if (intersectEdgeEdge(tri1[i], tri1[(i+1)%3],
                                tri2[i], tri2[(i+1)%3],
                                hints)) {
                hints.triangle_triangle.type = Hints::TriTri::EDGE_EDGE;
                hints.triangle_triangle.a = i;
                hints.triangle_triangle.b = j;
                return true;
            }
        }
    } catch (std::exception & e) {
        ls_warning("Had a calculation problem. Must divide time.\n");
        hints.triangle_triangle.must_divide_time = true;
        return true;
    }
    return false;
}

} // namespace Collide
