#ifndef TNL_SIMPLECAMERA_H
#define TNL_SIMPLECAMERA_H

#include <interfaces/ICamera.h>
#include <modules/jogi/JCamera.h>
#include <modules/math/Plane.h>

class SimpleCameraBase: public ICamera
{
protected:
    SimpleCameraBase();
    ~SimpleCameraBase();
    
public:
    void setFocus(float val);
    void setAspect(float val);
    void setNearDistance(float val);
    void setFarDistance(float val);

    // ICamera methods
    virtual Vector getFrontVector();
    virtual Vector getRightVector();
    virtual Vector getUpVector();

    virtual float getFocus();
    virtual float getAspect();
    virtual float getNearDistance();
    virtual float getFarDistance();

    virtual Matrix3 getOrient();
    virtual Matrix3 getOrientInv();
    virtual void getCamera(JCamera *cam);
    virtual void getFrontBackPlane(float plane[4]);
    virtual void getFrustumPlanes(float planes[6][4]);
    
    // Derived classes must implement:
    // getOrientation, getLocation, getMovementVector
protected:
    float focus, aspect;
    float near_dist, far_dist;
};

class SimpleCamera: public SimpleCameraBase
{
    Vector location, velocity;
    Matrix3 orient;

public:
    /// Creates a new camera with default value
    SimpleCamera();
    /// Creates a new camera with values copied from other.
    SimpleCamera(Ptr<ICamera> other);

    void alignWith(IMovementProvider *provider);
    
    SimpleCamera & operator= (ICamera & other);
    
    // IPositionProvider methods
    virtual Vector getLocation();
    virtual void getOrientation(Vector *up, Vector *right, Vector *front);
    
    // IMovementProvider methods
    virtual Vector getMovementVector();
    
    // IPositionReceiver methods
    virtual void setLocation(const Vector &p);
    virtual void setOrientation(const Vector & up,
    							const Vector & right,
    							const Vector & front);
    							
    // IMovementReceiver methods
    virtual void setMovementVector(const Vector& velocity);
};


#endif

