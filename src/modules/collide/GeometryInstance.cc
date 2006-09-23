#include "GeometryInstance.h"

namespace Collide {

GeometryInstance::GeometryInstance()
{
    collidable = 0;
    transforms_0 = new Transform[0];
    transforms_1 = new Transform[0];
}

GeometryInstance::GeometryInstance(Ptr<Collidable> c)
:   collidable(c)
{
    int n = c->getBoundingGeometry()->getNumOfTransforms();
    transforms_0 = new Transform[n];
    transforms_1 = new Transform[n];
}

GeometryInstance::GeometryInstance(const GeometryInstance & g)
:   collidable(g.collidable)
{
    int n;
    if(collidable) {
        n = collidable->getBoundingGeometry()->getNumOfTransforms();
    } else n = 0;
    transforms_0 = new Transform[n];
    transforms_1 = new Transform[n];
    for(int i=0; i<n; i++) {
        transforms_0[i] = g.transforms_0[i];
        transforms_1[i] = g.transforms_1[i];
    }
}

GeometryInstance::~GeometryInstance() {
    delete [] transforms_0;
    delete [] transforms_1;
}

GeometryInstance &
GeometryInstance::operator=(const GeometryInstance & g)
{
    delete [] transforms_0;
    delete [] transforms_1;

    collidable = g.collidable;
    int n;
    if(collidable) {
        n = collidable->getBoundingGeometry()->getNumOfTransforms();
    } else n = 0;
    transforms_0 = new Transform[n];
    transforms_1 = new Transform[n];
    for(int i=0; i<n; i++) {
        transforms_0[i] = g.transforms_0[i];
        transforms_1[i] = g.transforms_1[i];
    }
    
    return *this;
}

} // namespace Collide
