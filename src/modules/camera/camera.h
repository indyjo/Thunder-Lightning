#include <tnl.h>
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <interfaces/IGame.h>
#include <modules/jogi/JCamera.h>
#include <modules/math/Plane.h>

class Camera: public ICamera, public SigObject
{
public:
    Camera(IGame *thegame);
    
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

    void zoomIn();
    void zoomOut();
        
    void update();

private:

    IGame *thegame;
    Ptr<IConfig> config;
    float focus, aspect;
    Vector location;
    Matrix3 orient, orient_inv;
    Plane planes[6];
    JCamera jcam;
    bool dirty;
};
