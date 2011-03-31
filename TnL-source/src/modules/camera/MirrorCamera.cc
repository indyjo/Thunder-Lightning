#include <modules/math/SpecialMatrices.h>
#include "MirrorCamera.h"

MirrorCamera::~MirrorCamera()
{ }

Vector MirrorCamera::getLocation() {
    Vector p=target->getLocation();
    p[1] = -p[1];
    return p;
}

void MirrorCamera::getOrientation(Vector *up, Vector *right, Vector *front) {
    target->getOrientation(up, right, front);
    
    (*right)[1] = -(*right)[1];
    (*front)[1] = -(*front)[1];
    *up = (*front) % (*right);
}

Vector MirrorCamera::getMovementVector() {
    Vector v=target->getMovementVector();
    v[1] = -v[1];
    return v;
}

Matrix3 MirrorCamera::camToMir() {
    Vector right, up, front;

    target->getOrientation(&up, &right, &front);
    Matrix3 cam_orient = MatrixFromColumns(right, up, front);
    
    getOrientation(&up, &right, &front);
    Matrix3 mir_orient = MatrixFromColumns(right, up, front);
    
    return mir_orient.transpose() * cam_orient;
}

float MirrorCamera::getFocus() { return target->getFocus(); }
float MirrorCamera::getAspect() { return target->getAspect(); }
float MirrorCamera::getNearDistance() { return target->getNearDistance(); }
float MirrorCamera::getFarDistance() { return target->getFarDistance(); }


