#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <iostream>
#include <string>
#include <vector>
#include <landscape.h>


namespace Collide {

struct BoundingBox {
    Vector pos;
    float dim[3];
};

std::ostream & operator<< (std::ostream & out, const BoundingBox & bb);
std::istream & operator>> (std::istream & in, BoundingBox & bb);

} // namespace Collide

#endif
