#ifndef FOLLOWING_CAMERA_H
#define FOLLOWING_CAMERA_H

#include "SimpleCamera.h"

class FollowingCamera : public SimpleCameraBase {
    Ptr<IMovementProvider> target;
    Ptr<ICamera> camera;
    bool own_focus, own_aspect, own_near_dist, own_far_dist;
public:
    inline FollowingCamera()
    : own_focus(true)
    , own_aspect(true)
    , own_near_dist(true)
    , own_far_dist(true)
    { }
    ~FollowingCamera();
    
    inline Ptr<IMovementProvider> getTarget() { return target; }
    inline void setTarget(Ptr<IMovementProvider> t) { target = t; camera = 0; }
    void setTarget(Ptr<ICamera> c) { target = c; camera = c; }
    
    virtual Vector getLocation();
    virtual void getOrientation(Vector *up, Vector *right, Vector *front);
    virtual Vector getMovementVector();

    virtual float getFocus();
    virtual float getAspect();
    virtual float getNearDistance();
    virtual float getFarDistance();
    
    inline void ownFocus(bool b)         { own_focus = b; }
    inline void ownAspect(bool b)        { own_aspect = b; }
    inline void ownNearDistance(bool b)  { own_near_dist = b; }
    inline void ownFarDistance(bool b)   { own_far_dist = b; }
};

#endif

