#ifndef TNL_SIMPLECAMERA_H
#define TNL_SIMPLECAMERA_H

#include <interfaces/ICamera.h>
#include <modules/jogi/JCamera.h>
#include <modules/math/Plane.h>

class SimpleCamera: public ICamera
{
    bool dirty;
    float focus, aspect;
    Vector location;
    Matrix3 orient, orient_inv;
    Plane planes[6];
    JCamera jcam;
    float near_dist, far_dist;

public:
    /// Creates a new camera with default value
    SimpleCamera();
    /// Creates a new camera with values copied from other.
    SimpleCamera(Ptr<ICamera> other);
    
    // SimpleCamera methods
    void setFocus(float val);
    void setAspect(float val);
    void setNearDistance(float val);
    void setFarDistance(float val);
    
    // IPositionProvider methods
    virtual Vector getLocation();
    virtual Vector getFrontVector();
    virtual Vector getRightVector();
    virtual Vector getUpVector();
    virtual void getOrientation(Vector *up, Vector *right, Vector *front);
    
    // IPositionReceiver methods
    virtual void setLocation(const Vector &p);
    virtual void setOrientation(const Vector & up,
    							const Vector & right,
    							const Vector & front);

    // ICamera methods
    virtual void alignWith(IPositionProvider *pos_provider);

    virtual void getCamera(JCamera *cam);
    
    virtual void getFrontBackPlane(float plane[4]);
    virtual void getFrustumPlanes(float planes[6][4]);
    virtual float getFocus();
    virtual float getAspect();
    
    virtual const Matrix3 & getOrient();
    virtual const Matrix3 & getOrientInv();
    virtual float getNearDistance();
    virtual float getFarDistance();
    

private:
    void update();
};


#endif

