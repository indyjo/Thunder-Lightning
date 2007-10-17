#include <tnl.h>
#include <interfaces/IGame.h>
#include <SDL.h>
#include <modules/jogi/JOpenGLRenderer.h>
#include <remap.h>
#include <ActorStage.h>
#include <DataNode.h>
#include <RenderPass.h>
#include <Weak.h>

class Status;
struct ILoDQuadManager;
struct ISkyBox;
class IoScriptingManager;
namespace UI {
	class Console;
	class MainGUI;
}
class Water;
struct RenderContext;
class Camera;

class Game: public IGame, public ActorStage, public SigObject
{
public:
    static Game * the_game;

    Game(int argc, const char **argv);
    ~Game();

    void run();
    
    virtual void renderScene(SceneRenderPass *) ;
    virtual const RenderContext *getCurrentContext();
    virtual Ptr<SceneRenderPass> createRenderPass(Ptr<IMovementProvider> view_head);

    virtual Ptr<TextureManager> getTexMan();
    virtual JRenderer *getRenderer();
    virtual Ptr<EventRemapper> getEventRemapper();
    virtual Ptr<IModelMan> getModelMan();
    virtual Ptr<IConfig> getConfig();
    virtual Ptr<ICamera> getCamera();
    virtual UI::Surface getScreenSurface();
    virtual Ptr<Clock> getClock();
    virtual Ptr<ITerrain> getTerrain();
    virtual Ptr<Environment> getEnvironment();
    virtual Ptr<Water> getWater();
    virtual Ptr<IFontMan> getFontMan();
    virtual Ptr<SoundMan> getSoundMan();
    virtual Ptr<Collide::CollisionManager> getCollisionMan();
    virtual Ptr<IoScriptingManager> getIoScriptingManager();
    virtual void infoMessage(const char * msg, const Vector color);
    
    virtual double  getTimeDelta();
    virtual bool debugMode();
    virtual Ptr<DataNode> getDebugData();
    
    virtual Ptr<IView> getCurrentView();
    virtual void setCurrentView(Ptr<IView>);
    
    virtual Ptr<IActor> getCurrentlyControlledActor();
    virtual void setCurrentlyControlledActor(Ptr<IActor>);
    
    virtual void restartSimulation();
    
    virtual void clearScreen();
    virtual void endGame();

    virtual bool isInteractive();
    virtual void setInteractive(bool);

private:
    void startupSystem(Status &);
    void teardownSystem(Status &);
    
    void startupSimulation(Status &);
    void teardownSimulation(Status &);

    void initControls();

    void doEvents();

    void setupRenderer();
    void updateSimulation();
    void updateView();
    void updateSound();
    void setupMainRender();
    void updateIoScripting();
    
    void doFrame();

    void mainMenu();

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
    bool is_interactive;
    Ptr<DataNode> debug_data;

    SDL_Surface *surface;
    JOpenGLRenderer *renderer;
    const RenderContext *render_context;

    Ptr<TextureManager> texman;

    Ptr<EventRemapper> event_remapper;
    Ptr<EventSheet> event_sheet;

    Ptr<IView> current_view;
    Ptr<IView> previous_view;
    bool view_is_external;
    Ptr<IActor> current_actor;
    
    Ptr<Camera> camera;
    Ptr<Clock> clock;
    Ptr<IConfig> config;
    Ptr<IFontMan> fontman;
    Ptr<SoundMan> soundman;
    Ptr<IModelMan> modelman;
    Ptr<Collide::CollisionManager> collisionman;
    Ptr<ILoDQuadManager> quadman;
    Ptr<ISkyBox> skybox;
    Ptr<Environment> environment;
    Ptr<Water> water;
	Ptr<IoScriptingManager> io_scripting_manager;
    Ptr<UI::Console> console;
    Ptr<RenderPass> renderpass_main;
    Ptr<RenderPass> renderpass_overlay;
    Ptr<UI::MainGUI> main_gui;
};

