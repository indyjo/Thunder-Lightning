#ifndef ICAMERA_H
#define ICAMERA_H

#include <modules/math/Matrix.h>
#include "IPositionProvider.h"
#include "IPositionReceiver.h"

class JCamera;

class ICamera: virtual public IPositionProvider,
			   virtual public IPositionReceiver
{
public:
    virtual void alignWith(IPositionProvider *pos_provider)=0;

    virtual void getCamera(JCamera *camera)=0;

    virtual void getFrontBackPlane(float plane[4])=0;
    virtual void getFrustumPlanes(float planes[6][4])=0;
    virtual float getFocus()=0;
    virtual float getAspect()=0;
    
    virtual const Matrix3 & getOrient()=0;
    virtual const Matrix3 & getOrientInv()=0;
};

#endif
