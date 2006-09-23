#ifndef COLLIDE_BOUNDINGNODE_H
#define COLLIDE_BOUNDINGNODE_H

#include <iostream>
#include <modules/math/Vector.h>
#include "BoundingBox.h"

namespace Collide {

struct BoundingNode {
    inline BoundingNode() : type(NONE) { }
    inline ~BoundingNode() { cleanup(); }
    void cleanup();

    enum { NONE,
           LEAF,
           INNER,
           NEWDOMAIN,
           TRANSFORM } type;
    BoundingBox box;
    union {
        struct {
            int n_triangles;
            Vector * vertices;
        } leaf;
        struct {
            BoundingNode * children[2];
        } inner;
        struct {
            int domain_id;
            BoundingNode * child;
        } domain;
        struct {
            int transform_id;
            BoundingNode * child;
        } transform;
    } data;
};

std::ostream & operator<< (std::ostream & out, const BoundingNode & bn);
std::istream & operator>> (std::istream & in, BoundingNode & bn);

} // namespace Collide

#endif
