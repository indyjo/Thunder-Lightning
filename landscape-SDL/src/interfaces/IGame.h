#ifndef IGAME_H
#define IGAME_H

#include <object.h>
#include <interfaces/IActorStage.h>

class TextureManager;
class JRenderer;
class EventRemapper;
class IModelMan;
class IConfig;
class IActor;
class ICamera;
class Clock;
class ITerrain;
class IPlayer;
class IGunsight;
class Environment;
class IFontMan;
class SoundMan;
namespace Collide {
	class CollisionManager;
}

class IGame : virtual public Object, virtual public IActorStage
{
public:
    virtual Ptr<TextureManager> getTexMan()=0;
    virtual JRenderer *getRenderer()=0;
    virtual EventRemapper *getEventRemapper()=0;
    virtual Ptr<IModelMan> getModelMan()=0;
    virtual Ptr<IConfig> getConfig()=0;
    virtual Ptr<IActor> getCamPos()=0;
    virtual Ptr<ICamera> getCamera()=0;
    virtual Ptr<Clock> getClock()=0;
    virtual Ptr<ITerrain> getTerrain()=0;
    virtual Ptr<IPlayer> getPlayer()=0;
    virtual Ptr<IGunsight> getGunsight()=0;
    virtual Ptr<Environment> getEnvironment()=0;
    virtual Ptr<IFontMan> getFontMan()=0;
    virtual Ptr<SoundMan> getSoundMan()=0;
    virtual Ptr<Collide::CollisionManager> getCollisionMan()=0;
    virtual void getMouseState(float *mx, float *my, int *buttons)=0;
    virtual double  getTimeDelta()=0;
    virtual double  getTime()=0;
    virtual void drawDebugTriangleAt(const Vector & p) = 0;

    virtual void clearScreen()=0;
};


#endif
