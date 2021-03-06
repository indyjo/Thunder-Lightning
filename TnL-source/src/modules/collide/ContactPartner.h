#ifndef COLLIDE_CONTACTPARTNER_H
#define COLLIDE_CONTACTPARTNER_H

#include <modules/math/Vector.h>
#include "GeometryInstance.h"

namespace Collide {

struct BoundingNode;
class BoundingGeometry;

struct ContactPartner {
    GeometryInstance * instance;
    int domain, transform;
    enum { TRIANGLE, NODE, GEOM } type;
    union {
        const Vector           * triangle;
        const BoundingNode     * node;
        const BoundingGeometry * geom;
    } data;

    inline ContactPartner() { }

    inline ContactPartner(GeometryInstance * g, Vector * t, int dom=0, int tr=0)
    :   instance(g), type(TRIANGLE),
        domain(dom), transform(tr)
    {
        data.triangle = t;
    }

    inline ContactPartner(GeometryInstance * g, BoundingNode * n, int dom=0, int tr=0)
    :   instance(g), type(NODE),
        domain(dom), transform(tr)
    {
        data.node = n;
    }

    inline ContactPartner(GeometryInstance * g, BoundingGeometry * b, int dom=0, int tr=0)
    :   instance(g), type(GEOM),
        domain(dom), transform(tr)
    {
        data.geom = b;
    }

    inline bool isTriangle() { return type == TRIANGLE; }
    inline bool isNode() { return type == NODE; }
    inline bool isSphere() { return type == GEOM; }
    inline bool canSubdivide() {
        if (isTriangle()) return false;
        else if (isNode()) return data.node->type != BoundingNode::NONE;
        else return data.geom->getRootNode()->type != BoundingNode::NONE;
    }
    inline bool mustSubdivide() {
        return isNode() && !data.node->isValidBoundingBox();
    }
    inline float volume() {
        switch(type) {
            case GEOM:
            {
                float r = data.geom->getBoundingRadius();
                return 4*3.141593f*r*r*r/3;
            }
            case NODE:
                return data.node->box.dim[0]*data.node->box.dim[1]*data.node->box.dim[2];
            case TRIANGLE:
                return 0;
        }
    }
};

} // namespace Collide

#endif
