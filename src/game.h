#include <tnl.h>
#include <interfaces/IGame.h>
#include <SDL.h>
#include <modules/jogi/JOpenGLRenderer.h>
#include <remap.h>
#include <ActorStage.h>

class Status;
struct ILoDQuadManager;
struct ISkyBox;
class IoScriptingManager;
namespace UI {
	class Console;
}
class Water;

class Game: public IGame, public ActorStage, public SigObject
{
public:
    static Game * the_game;

    Game(int argc, const char **argv);
    ~Game();

    void run();

    virtual Ptr<TextureManager> getTexMan();
    virtual JRenderer *getRenderer();
    virtual Ptr<EventRemapper> getEventRemapper();
    virtual Ptr<IModelMan> getModelMan();
    virtual Ptr<IConfig> getConfig();
    virtual Ptr<ICamera> getCamera();
    virtual UI::Surface getScreenSurface();
    virtual Ptr<Clock> getClock();
    virtual Ptr<ITerrain> getTerrain();
    virtual Ptr<IDrawable> getGunsight();
    virtual void setGunsight(Ptr<IDrawable>);
    virtual Ptr<Environment> getEnvironment();
    virtual Ptr<IFontMan> getFontMan();
    virtual Ptr<SoundMan> getSoundMan();
    virtual Ptr<Collide::CollisionManager> getCollisionMan();
    virtual Ptr<IoScriptingManager> getIoScriptingManager();
    virtual void infoMessage(const char * msg, const Vector color);
    
    virtual double  getTimeDelta();
    virtual void drawDebugTriangleAt(const Vector & p);
    virtual bool debugMode();

    
    virtual Ptr<IView> getCurrentView();
    virtual void setCurrentView(Ptr<IView>);
    
    virtual Ptr<IActor> getCurrentlyControlledActor();
    virtual void setCurrentlyControlledActor(Ptr<IActor>);
    
    virtual void clearScreen();

private:
    void initModules(Status &);

    void initControls();

    void doEvents();

    void setupRenderer();
    void updateSimulation();
    void updateView();
    void updateSound();
    void setupMainRender();
    void setupMirroredRender();
    void updateIoScripting();
    
    void doFrame();

    void drawDebugTriangle();

    void endGame();

    void actionTriggered(const char *);

public:
    void togglePauseMode();
    void toggleDebugMode();
private:
    void toggleFollowMode();

    void accelerateSpeed();
    void decelerateSpeed();
    
    void setView(int);
    void externalView();
    
    void nextTarget();
    void toggleControlMode();

private:
    int argc;
    const char **argv;
    
    bool game_done;
    bool debug_mode;

    SDL_Surface *surface;
    JOpenGLRenderer *renderer;

    Ptr<TextureManager> texman;

    Ptr<EventRemapper> event_remapper;

    Ptr<IView> current_view;
    Ptr<IView> previous_view;
    bool view_is_external;
    Ptr<IActor> current_actor;
    
    Ptr<ICamera> camera;
    Ptr<Clock> clock;
    Ptr<IConfig> config;
    Ptr<IFontMan> fontman;
    Ptr<SoundMan> soundman;
    Ptr<IModelMan> modelman;
    Ptr<Collide::CollisionManager> collisionman;
#if ENABLE_LOD_TERRAIN
    Ptr<ILoDQuadManager> quadman;
#endif
#if ENABLE_SKYBOX
    Ptr<ISkyBox> skybox;
    Ptr<IDrawable> gunsight;
#endif
    Ptr<Environment> environment;
    Ptr<Water> water;
	Ptr<IoScriptingManager> io_scripting_manager;
    Ptr<UI::Console> console;
};

