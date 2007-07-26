#ifndef MIRROR_CAMERA_H
#define MIRROR_CAMERA_H

#include "SimpleCamera.h"

class MirrorCamera : public SimpleCameraBase {
    Ptr<ICamera> target;
public:
    inline MirrorCamera(Ptr<ICamera> target)
    : target(target)
    { }
    ~MirrorCamera();
    
    inline Ptr<ICamera> getTarget() { return target; }
    inline void setTarget(Ptr<ICamera> t) { target = t; }
    
    Matrix3 camToMir();
    
    virtual Vector getLocation();
    virtual void getOrientation(Vector *up, Vector *right, Vector *front);
    virtual Vector getMovementVector();

    virtual float getFocus();
    virtual float getAspect();
    virtual float getNearDistance();
    virtual float getFarDistance();
};

#endif

