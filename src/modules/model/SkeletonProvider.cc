#include "SkeletonProvider.h"

SkeletonProvider::SkeletonProvider(Ptr<Skeleton> skel, const char *p, const char *f1, const char *f2,
                                   const char* u1, const char *u2)
    :   skeleton(skel), pos(p), front1(f1), front2(f2), up1(u1), up2(u2)
{ }
    
Vector SkeletonProvider::getLocation() { return skeleton->getPoint(pos); }
    
Matrix3 SkeletonProvider::orient() {
    Vector front = skeleton->getPoint(front2)-skeleton->getPoint(front1);
    Vector right, up;
    if (up1) {
        up = skeleton->getPoint(up2)-skeleton->getPoint(up1);
        right = up % front;
    } else {
        right = Vector(0,1,0) % front;
        right.normalize();
        up = front % right;
    }
    return MatrixFromColumns(right,up,front);
}

Vector SkeletonProvider::getFrontVector() {
    return skeleton->getPoint(front2)-skeleton->getPoint(front1);
}

Vector SkeletonProvider::getRightVector() {
    return orient()*Vector(1,0,0);
}

Vector SkeletonProvider::getUpVector() {
    return orient()*Vector(0,1,0);
}

void SkeletonProvider::getOrientation(Vector *up, Vector *right, Vector *front) {
    Matrix3 M = orient();
    *up = M * Vector(0,1,0);
    *right = M * Vector(1,0,0);
    *front = M * Vector(0,0,1);
}
