#ifndef IGAME_H
#define IGAME_H

#include <object.h>
#include <interfaces/IActorStage.h>
#include <modules/ui/Surface.h>

class TextureManager;
class JRenderer;
class EventRemapper;
class Clock;
class Environment;
class SoundMan;
class IoScriptingManager;

class IActor;
class IConfig;
class ICamera;
class IDrawable;
class IFontMan;
class IModelMan;
class IPlayer;
class ITerrain;
class IView;

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
    virtual Ptr<ICamera> getCamera()=0;
    virtual UI::Surface getScreenSurface()=0;
    virtual Ptr<Clock> getClock()=0;
    virtual Ptr<ITerrain> getTerrain()=0;
    virtual Ptr<IDrawable> getGunsight()=0;
    virtual void setGunsight(Ptr<IDrawable>)=0;
    virtual Ptr<Environment> getEnvironment()=0;
    virtual Ptr<IFontMan> getFontMan()=0;
    virtual Ptr<SoundMan> getSoundMan()=0;
    virtual Ptr<Collide::CollisionManager> getCollisionMan()=0;
    virtual Ptr<IoScriptingManager> getIoScriptingManager()=0;
    virtual void getMouseState(float *mx, float *my, int *buttons)=0;
    virtual double  getTimeDelta()=0;
    virtual double  getTime()=0;
    virtual void drawDebugTriangleAt(const Vector & p) = 0;
    
    virtual Ptr<IView> getCurrentView()=0;
    virtual void setCurrentView(Ptr<IView>)=0;
    
    virtual Ptr<IActor> getCurrentlyControlledActor()=0;
    virtual void setCurrentlyControlledActor(Ptr<IActor>)=0;
    
    virtual void clearScreen()=0;
};


#endif
