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

struct IActor;
struct IConfig;
struct ICamera;
struct IDrawable;
struct IFontMan;
struct IModelMan;
struct IPlayer;
struct ITerrain;
struct IView;

namespace Collide {
	class CollisionManager;
}

struct IGame : virtual public Object, virtual public IActorStage
{
    virtual Ptr<TextureManager> getTexMan()=0;
    virtual JRenderer *getRenderer()=0;
    virtual Ptr<EventRemapper> getEventRemapper()=0;
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
    
    /// Display a user-visible message in the given color.
    /// @note For the message to show up, the current gunsight
    ///       must have an InfoMessage module.
    virtual void infoMessage(const char * msg, const Vector color=Vector(1,1,1))=0;
    
    typedef SigC::Signal2<void, const char *, const Vector&> InfoMessageSignal;
    /// Allow listeners to hook into infoMessages
    InfoMessageSignal info_message_signal;
    
    /// Obsolete time delta function. Replaced by Clock.
    virtual double  getTimeDelta()=0;
    
    /// true if program-wide debug mode is enabled
    virtual bool debugMode()=0;
    
    virtual Ptr<IView> getCurrentView()=0;
    virtual void setCurrentView(Ptr<IView>)=0;
    
    virtual Ptr<IActor> getCurrentlyControlledActor()=0;
    virtual void setCurrentlyControlledActor(Ptr<IActor>)=0;
    
    virtual void clearScreen()=0;
};


#endif
