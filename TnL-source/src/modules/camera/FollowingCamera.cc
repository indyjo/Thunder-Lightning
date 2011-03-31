#include "FollowingCamera.h"

FollowingCamera::~FollowingCamera()
{ }

Vector FollowingCamera::getLocation() {
    return target->getLocation();
}

void FollowingCamera::getOrientation(Vector *up, Vector *right, Vector *front) {
    target->getOrientation(up, right, front);
}

Vector FollowingCamera::getMovementVector() {
    return target->getMovementVector();
}

float FollowingCamera::getFocus() { return own_focus?focus:camera->getFocus(); }
float FollowingCamera::getAspect()  { return own_aspect?aspect:camera->getAspect(); }
float FollowingCamera::getNearDistance()  { return own_near_dist?near_dist:camera->getNearDistance(); }
float FollowingCamera::getFarDistance()  { return own_far_dist?far_dist:camera->getFarDistance(); }

