#ifndef IGAME_H
#define IGAME_H

#include <object.h>
#include <interfaces/IActorStage.h>
#include <modules/ui/Surface.h>
#include <Weak.h>

class TextureManager;
class JRenderer;
class EventRemapper;
class Clock;
class DataNode;
class Environment;
class SoundMan;
#ifdef HAVE_IO
class IoScriptingManager;
#endif
class Water;

struct IActor;
struct IConfig;
struct ICamera;
struct IDrawable;
struct IFontMan;
struct IModelMan;
struct IPlayer;
struct ITerrain;
struct IView;

struct RenderContext;
class RenderPass;
class SceneRenderPass;

namespace Collide {
	class CollisionManager;
}

struct IGame : virtual public Object, virtual public IActorStage, virtual public Weak
{
    /// Called by the SceneRenderPass with itself as an argument.
    /// Sets the current context and camera and renders a scene according
    /// to the configuration of the context.
    virtual void renderScene(SceneRenderPass *) = 0;
    
    /// During rendering, a RenderContext is active.
    /// @return the currently active RenderContext
    virtual const RenderContext *getCurrentContext() = 0;
    
    /// Create a SceneRenderPass with default values and a camera configured
    /// to follow view_head.
    /// While it is possible to create a RenderPass manually, this is usually
    /// the easiest method. The camera's aspect and focal length will be taken
    /// from config.
    virtual Ptr<SceneRenderPass> createRenderPass(Ptr<IMovementProvider> view_head)=0;

    virtual Ptr<TextureManager> getTexMan()=0;
    virtual JRenderer *getRenderer()=0;
    virtual Ptr<EventRemapper> getEventRemapper()=0;
    virtual Ptr<IModelMan> getModelMan()=0;
    virtual Ptr<IConfig> getConfig()=0;
    virtual Ptr<ICamera> getCamera()=0;
    virtual UI::Surface getScreenSurface()=0;
    virtual Ptr<Clock> getClock()=0;
    virtual Ptr<ITerrain> getTerrain()=0;
    virtual Ptr<Environment> getEnvironment()=0;
    virtual Ptr<Water> getWater()=0;
    virtual Ptr<IFontMan> getFontMan()=0;
    virtual Ptr<SoundMan> getSoundMan()=0;
    virtual Ptr<Collide::CollisionManager> getCollisionMan()=0;
#ifdef HAVE_IO
    virtual Ptr<IoScriptingManager> getIoScriptingManager()=0;
#endif
    
    /// Display a user-visible message in the given color.
    /// @note For the message to show up, the current gunsight
    ///       must have an InfoMessage module.
    virtual void infoMessage(const char * msg, const Vector color=Vector(1,1,1))=0;
    
    typedef SigC::Signal2<void, const char *, const Vector&> InfoMessageSignal;
    /// Allow listeners to hook into infoMessages
    InfoMessageSignal info_message_signal;
    
    typedef SigC::Signal0<void> DrawSignal;
    DrawSignal pre_draw, post_draw;
    
    /// Obsolete time delta function. Replaced by Clock.
    virtual double  getTimeDelta()=0;
    
    /// true if program-wide debug mode is enabled
    virtual bool debugMode()=0;
    /// A flexible interface for passing profiling info, statistics and debug data to the Io side
    virtual Ptr<DataNode> getDebugData()=0;
    
    virtual Ptr<IView> getCurrentView()=0;
    virtual void setCurrentView(Ptr<IView>)=0;
    
    virtual Ptr<IActor> getCurrentlyControlledActor()=0;
    virtual void setCurrentlyControlledActor(Ptr<IActor>)=0;
    
    /// This will restart the simulation, reset the running simulation and
    /// invalidate all simulation-specific variables.
    virtual void restartSimulation()=0;
    
    virtual void clearScreen()=0;
    virtual void endGame()=0;
    
    /// Returns whether the simulation state is considered interactive
    virtual bool isInteractive()=0;
    /// Marks simulation as interactive (true) or non-interactive, with consequences
    /// for GUI behavior. An interactive simulation will allow games to be saved and
    /// will warn on exit. A non-interactive simulation could be a demo 
    virtual void setInteractive(bool)=0;
};


#endif
