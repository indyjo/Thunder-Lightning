#ifndef ICAMERA_H
#define ICAMERA_H

#include <modules/math/Matrix.h>
#include "IMovementProvider.h"

class JCamera;

#define PLANE_MINUS_Z 0
#define PLANE_PLUS_Z 1
#define PLANE_MINUS_X 2
#define PLANE_PLUS_X 3
#define PLANE_MINUS_Y 4
#define PLANE_PLUS_Y 5

struct ICamera: virtual public IMovementProvider
{
    virtual void getCamera(JCamera *camera)=0;

    virtual void getFrontBackPlane(float plane[4])=0;
    virtual void getFrustumPlanes(float planes[6][4])=0;
    virtual float getFocus()=0;
    virtual float getAspect()=0;
    virtual float getNearDistance()=0;
    virtual float getFarDistance()=0;
    
    virtual Matrix3 getOrient()=0;
    virtual Matrix3 getOrientInv()=0;
};

#endif
