#include <modules/math/Collide.h>

#include "Contact.h"
#include "Primitive.h"
#include "PossibleContact.h"

namespace {
template<class T>
XTransform<T> get_transform(const T & u, int i,
                            Collide::GeometryInstance * instance)
{
    Ptr<Collide::BoundingGeometry> geom =
        instance->collidable->getBoundingGeometry();
    XTransform<T> transform = interp(u,
        (XTransform<T>) instance->transforms_0[i],
        (XTransform<T>) instance->transforms_1[i]);
    int j;
    while (i != (j=geom->getParentOfTransform(i))) {
        i = j;
        transform = interp(u,
            (XTransform<T>) instance->transforms_0[i],
            (XTransform<T>) instance->transforms_1[i])
            * transform;
    }
    return transform;
}

/*
template<class T>
XVector<3,T> get_velocity(XVector<3,T> x, const T & u, int i,
                          Collide::GeometryInstance * instance)
{
    Ptr<Collide::BoundingGeometry> geom =
        instance->collidable->getBoundingGeometry();
    XVector<3,T> a(1,1,1), b(1,1,1), c;

    XTransform<T> transform = interp(u,
        (XTransform<T>) instance->transforms_0[i],
        (XTransform<T>) instance->transforms_1[i]);
    b = transform(x);
    int j;
    while ((j=geom->getParentOfTransform(i)) != i) {
        c = get_derivative(i, u, instamce)(b);
        for(int k=0; k<3; k++) a[k] *= c[k];
        transform = interp(u,
            (XTransform<T>) instance->transforms_0[i],
            (XTransform<T>) instance->transforms_1[i]);
        for(int k=0; k<3; k++)
*/

} // namespace


namespace Collide {
	
bool PossibleContact::mustSubdivide() {
    if (partners[0].isTriangle() && partners[1].isBox()) return true;
    if (partners[0].isBox() && partners[1].isTriangle()) return true;
    return false;
}

void PossibleContact::subdivide(std::priority_queue<PossibleContact> & q) {
    if (!partners[1].canSubdivide())
        std::swap(partners[0], partners[1]);
    // We can assume that it is possible to subdivide partners[1] now!
    PossibleContact new_contact;
    new_contact.partners[1] = partners[0];
    new_contact.t0 = t0;
    new_contact.t1 = t1;
    new_contact.partners[0].instance = partners[1].instance;
    new_contact.partners[0].transform = partners[1].transform;
    new_contact.partners[0].domain = partners[1].domain;

    if (partners[1].isBox()) {
        const BoundingNode & node = *partners[1].data.node;
        switch(node.type) {
        case BoundingNode::LEAF:
            new_contact.partners[0].type = ContactPartner::TRIANGLE;
            for(int i=0; i<node.data.leaf.n_triangles*3; i+=3) {
                new_contact.partners[0].data.triangle =
                    & node.data.leaf.vertices[i];
                q.push(new_contact);
            }
            break;
        case BoundingNode::INNER:
            new_contact.partners[0].type = ContactPartner::NODE;
            for(int i=0; i<2; i++) {
                new_contact.partners[0].data.node = node.data.inner.children[i];
                q.push(new_contact);
            }
            break;
        case BoundingNode::NEWDOMAIN:
            new_contact.partners[0].type = ContactPartner::NODE;
            new_contact.partners[0].data.node = node.data.domain.child;
            new_contact.partners[0].domain = node.data.domain.domain_id;
            q.push(new_contact);
            break;
        case BoundingNode::TRANSFORM:
            new_contact.partners[0].type = ContactPartner::NODE;
            new_contact.partners[0].data.node = node.data.transform.child;
            new_contact.partners[0].transform = node.data.transform.transform_id;
            q.push(new_contact);
            break;
        case BoundingNode::NONE:
            break;
        }
    } else {
        // isSphere() == true
        new_contact.partners[0].type = ContactPartner::NODE;
        new_contact.partners[0].data.node = partners[1].data.geom->getRootNode();
        q.push(new_contact);
    }
}

bool PossibleContact::shouldDivideTime(const Hints & hints) {
    if (partners[0].isSphere()) {
        if (partners[1].isSphere())
            return false;
        if (partners[1].isBox())
            return hints.box.exactness > 0.5*hints.box.max_box_dim;
        if (partners[1].isTriangle())
            return hints.triangle_sphere.must_divide_time ||
                   hints.triangle_sphere.exactness > 0.5f;
    } else if (partners[0].isBox()) {
        if (partners[1].isSphere())
            return hints.box.exactness > 0.5*hints.box.max_box_dim;
        if (partners[1].isBox())
            return hints.box.exactness > 0.5*hints.box.max_box_dim;
        if (partners[1].isTriangle())
            return false;
    } else if (partners[0].isTriangle()) {
        if (partners[1].isSphere())
            return hints.triangle_sphere.must_divide_time ||
                   hints.triangle_sphere.exactness > 0.5f;
        if (partners[1].isBox())
            return false;
        if (partners[1].isTriangle())
            return hints.triangle_triangle.must_divide_time ||
                   hints.triangle_triangle.exactness > 0.5f;
    }

    // should never be called.
    ls_error("This should not happen!\n");
    ls_error("%d %d %d , %d %d %d\n",
        partners[0].isSphere(),partners[0].isBox(),partners[0].isTriangle(),
        partners[1].isSphere(),partners[1].isBox(),partners[1].isTriangle());
    return t1 - t0 > 0.001f;
}

void PossibleContact::divideTime(std::priority_queue<PossibleContact> & q) {
    float t_mid = 0.5f*(t0+t1);
    //ls_message("dividing time: %f-%f (new midpoint:%f)\n", t0, t1, t_mid);
    PossibleContact new_contact(*this);
    new_contact.t1 = t_mid;
    q.push(new_contact);
    new_contact.t0 = t_mid;
    new_contact.t1 = t1;
    q.push(new_contact);
}

bool PossibleContact::collide(float delta_t, Hints & hints) {
    if (t1-t0 < 1e-7) {
        ls_message("Time is equal! Cancelling collision test.\n");
        return false;
    }
    
    //ls_message("---------- %f - %f -----------\n", t0, t1);
    // Sphere / Sphere test
    if (partners[0].isSphere() && partners[1].isSphere()) {
        Vector p[2], v[2];
        float r[2];
        for(int i=0; i<2; i++) {
            p[i] = partners[i].instance->transforms_0[0].vec();
            v[i] = (partners[i].instance->transforms_1[0].vec() - p[i])
                   / delta_t;
            r[i] = partners[i].instance->collidable->
                getBoundingGeometry()->getBoundingRadius();
        }

        if (!canSubdivide() && (v[0]-v[1]) * (p[0] - p[1]) >= 0)
            return false;

        float c_t0, c_t1;
        if (! Collide::movingPoints(
            Line(p[0], v[0]), Line(p[1], v[1]), r[0], r[1],
            & c_t0, & c_t1))
        {
            return false;
        }

        if (c_t0 > c_t1) ls_error("c_t0 > c_t1!\n");

        if (c_t0 > t1 || c_t1 < t0)
            return false;

        t0 = std::max(t0, c_t0);
        t1 = std::min(t1, c_t1);

        return true;
    }

    if (partners[0].isSphere() && partners[1].isBox())
        std::swap(partners[0], partners[1]);
    if (partners[0].isSphere() && partners[1].isTriangle())
        std::swap(partners[0], partners[1]);

    ITransform T0 = get_transform(Interval(t0/delta_t, t1/delta_t),
        partners[0].transform, partners[0].instance);
    ITransform T1 = get_transform(Interval(t0/delta_t, t1/delta_t),
        partners[1].transform, partners[1].instance);

    if (partners[0].isBox() && partners[1].isBox()) {
        IVector orient0[3], orient1[3];
        orient0[0] = T0.quat().rot(IVector(1,0,0));
        orient0[1] = T0.quat().rot(IVector(0,1,0));
        orient0[2] = T0.quat().rot(IVector(0,0,1));
        orient1[0] = T1.quat().rot(IVector(1,0,0));
        orient1[1] = T1.quat().rot(IVector(0,1,0));
        orient1[2] = T1.quat().rot(IVector(0,0,1));

        // bounding box intersection test
        return intersectBoxBox(
            partners[0].data.node->box,
            T0((IVector) partners[0].data.node->box.pos), orient0,
            partners[1].data.node->box,
            T1((IVector) partners[1].data.node->box.pos), orient1,
            hints);
    }

    if (partners[0].isBox() && partners[1].isSphere()) {
        IVector orient0[3];
        orient0[0] = T0.quat().rot(IVector(1,0,0));
        orient0[1] = T0.quat().rot(IVector(0,1,0));
        orient0[2] = T0.quat().rot(IVector(0,0,1));

        // box / sphere intersection test
        return intersectBoxSphere(
            partners[0].data.node->box,
            T0((IVector) partners[0].data.node->box.pos), orient0,
            partners[1].data.geom->getBoundingRadius(), T1.vec(),
            hints);
    }

    if (partners[0].isTriangle() && partners[1].isSphere()) {
        // triangle / sphere intersection test
        return intersectTriangleSphere(
            partners[0].data.triangle, T0,
            partners[1].data.geom->getBoundingRadius(), T1.vec(),
            hints);
    }

    if (partners[0].isTriangle() && partners[1].isTriangle()) {
        // triangle / triangle intersection test
        return intersectTriangleTriangle(
            partners[0].data.triangle, T0,
            partners[1].data.triangle, T1,
            hints);
    }


    return false;
}

bool PossibleContact::makeContact(Contact & c, float delta_t,
                                  const Hints & hints)
{
    if (partners[0].isSphere() && partners[1].isTriangle()) {
        std::swap(partners[0], partners[1]);
    }

    Transform T0[2]= {
        get_transform(t0/delta_t, partners[0].transform, partners[0].instance),
        get_transform(t0/delta_t, partners[1].transform, partners[1].instance)};
    Transform T1[2]= {
        get_transform(t1/delta_t, partners[0].transform, partners[0].instance),
        get_transform(t1/delta_t, partners[1].transform, partners[1].instance)};

    Vector c_pos, normal, p[2], v[2];

    for(int i=0; i<2; i++) {
        p[i] = T0[i].vec();
    }

    if (partners[0].isSphere()) {
        float r[2];
        for(int i=0; i<2; i++)
            r[i] = partners[i].instance->collidable->
                getBoundingGeometry()->getBoundingRadius();

        c_pos = r[0] * (p[0] + t0*v[0]) + r[1] * (p[1] + t0*v[1]);
        c_pos /= r[0]+r[1];

        normal = p[0]-p[1];
        float l = normal.length();
        if (l < 1e-5) normal = Vector(1,0,0);
        else normal /= l;
    } else {
        if (partners[1].isSphere()) {
            const Vector & x(p[1]);
            if (hints.triangle_sphere.on_edge) {
                const int & edge = hints.triangle_sphere.edge;
                Vector a(T0[0](partners[0].data.triangle[edge]));
                Vector b(T0[0](partners[0].data.triangle[(edge+1)%3]));

                normal = (b-a) %((b-a) % (x-a));
                if (normal * (x-a) > 0)
                    normal = -normal;
                float l = normal.length();
                if (l < 1e-5) normal = Vector(1,0,0);
                else normal /= l;
                float d = (x-a) * normal;
                c_pos = x - d*normal;
            } else {
                Vector a(T0[0](partners[0].data.triangle[0]));
                Vector b(T0[0](partners[0].data.triangle[1]));
                Vector c(T0[0](partners[0].data.triangle[2]));

                normal = (b-a) % (c-a);

                if (normal * (x-a) > 0)
                    normal = -normal;
                float l = normal.length();
                if (l < 1e-5) normal = Vector(1,0,0);
                else normal /= l;
                float d = (x-a) * normal;
                c_pos = x - d*normal;
            }
        } else if (partners[1].isTriangle()) {
            if (hints.triangle_triangle.type ==
                Hints::TriTri::VERTEX_TRIANGLE) {
                int n = hints.triangle_triangle.a;
                Vector a(T0[1](partners[1].data.triangle[0]));
                Vector b(T0[1](partners[1].data.triangle[1]));
                Vector c(T0[1](partners[1].data.triangle[2]));
                Vector x(T0[0](partners[0].data.triangle[n]));

                normal = (b-a) % (c-a);
                float l = normal.length();
                if (l < 1e-5) normal = Vector(1,0,0);
                else normal /= l;

                Vector x2 = T0[0]( 0.5f *
                    (partners[0].data.triangle[(n+1) % 3]) +
                    (partners[0].data.triangle[(n+2) % 3]));
                x2 = x;
                if (normal * (x2-a) < 0)
                    normal = -normal;

                c_pos = x;
            } else if (hints.triangle_triangle.type ==
                Hints::TriTri::TRIANGLE_VERTEX) {
                const int & n = hints.triangle_triangle.b;
                Vector a(T0[0](partners[0].data.triangle[0]));
                Vector b(T0[0](partners[0].data.triangle[1]));
                Vector c(T0[0](partners[0].data.triangle[2]));
                Vector x(T0[1](partners[1].data.triangle[n]));

                normal = (b-a) % (c-a);
                float l = normal.length();
                if (l < 1e-5) normal = Vector(1,0,0);
                else normal /= l;

                Vector x2 = T0[1]( 0.5f *
                    (partners[1].data.triangle[(n+1) % 3]) +
                    (partners[1].data.triangle[(n+2) % 3]));
                x2 = x;
                if (normal * (x2-a) > 0)
                    normal = -normal;

                c_pos = x;
            } else { // edge / edge
                const int & edge0 = hints.triangle_triangle.a;
                const int & edge1 = hints.triangle_triangle.b;
                Vector a(T0[0](partners[0].data.triangle[edge0]));
                Vector b(T0[0](partners[0].data.triangle[(edge0+1)%3]));
                Vector c(T0[1](partners[1].data.triangle[edge1]));
                Vector d(T0[1](partners[1].data.triangle[(edge1+1)%3]));

                normal = (b-a) % (d-c);
                if (normal * (a-c) < 0)
                    normal = -normal;

                float l = normal.length();
                if (l < 1e-5) normal = Vector(1,0,0);
                else normal /= l;
                Vector edge_normal = normal % (b-a);
                float c_ab = (c-a) * edge_normal;
                float d_ab = (d-a) * edge_normal;
                if (c_ab-d_ab == 0.0) c_pos = c;
                else {
                    float t = c_ab / (c_ab-d_ab);
                    c_pos = c + t*(d-c);
                }
            }
        }
    }

    ls_message("p : %f %f %f\n",
        c_pos[0], c_pos[1], c_pos[2]);
    ls_message("n : %f %f %f\n",
        normal[0], normal[1], normal[2]);
    for(int i=0; i<2; i++) {
        Vector c_rel = T0[i].inv()(c_pos);
        v[i] = (T1[i](c_rel) - c_pos) / (t1-t0);
        ls_message("v[%d]: %f %f %f\n", i,
            v[i][0], v[i][1], v[i][2]);
    }

    if ((v[0]-v[1]) * normal >= 0) return false;

    for(int i=0; i<2; i++) {
        c.collidables[i] = partners[i].instance->collidable;
        c.domains[i]     = partners[i].domain;
        c.v[i]           = v[i] * normal;
    }

    c.p = c_pos;
    c.n = normal;

    return true;
}
	
	
}
