#include <landscape.h>

class Camera: public ICamera
{
public:
    Camera(IGame *thegame);
    
    // IPositionProvider methods
    virtual Vector getLocation();
    virtual Vector getFrontVector();
    virtual Vector getRightVector();
    virtual Vector getUpVector();
    virtual void getOrientation(Vector *up, Vector *right, Vector *front);

    // ICamera methods
    virtual void alignWith(IPositionProvider *pos_provider);

    virtual void getCamera(JCamera *camera);
    
    virtual void getFrontBackPlane(float plane[4]);
    virtual void getFrustumPlanes(float planes[6][4]);
    virtual float getFocus();
    virtual float getAspect();

private:
    IGame *thegame;
    Ptr<IConfig> config;
    float focus, aspect;
    float location[3];
    float front[3], right[3], up[3];
    float planes[6][4];
    JCamera cam;
};
