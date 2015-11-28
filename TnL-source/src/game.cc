#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <algorithm>
#include <sigc++/bind.h>
#include <glew.h>
#include <CEGUIDefaultResourceProvider.h>
#include <CEGUI.h>
#include <TnlCeguiLogger.h>
#include <RendererModules/OpenGL/CEGUIOpenGLRenderer.h>
#include <modules/math/Vector.h>
#include <modules/camera/camera.h>
#include <modules/camera/FollowingCamera.h>
#include <modules/clock/clock.h>
#include <modules/LoDTerrain/LoDTerrain.h>
#include <modules/skybox/skybox.h>
#include <modules/environment/environment.h>
#include <modules/environment/Water.h>
#include <modules/model/model.h>
#include <modules/model/modelman.h>
#include <modules/fontman/fontman.h>
#include <modules/gunsight/gunsight.h>
#include <modules/ui/loadingscreen.h>
#include <modules/ui/MainGUI.h>
#include <modules/collide/CollisionManager.h>
#include <modules/ui/Console.h>
#include <modules/ui/Surface.h>
#include <modules/ui/PanelRenderPass.h>
#include <modules/scripting/IoScriptingManager.h>
#include <modules/scripting/mappings.h>
#include <modules/weaponsys/RadarNet.h>
#include <modules/actors/Observer.h>
#include <modules/config/config.h>
#include <SceneRenderPass.h>
#include <sound_openal.h>
#include <Faction.h>

#include <SDL_main.h>
#include "game.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
    #define READ_PATH_FROM_ARGV0
    #define PREFIX_IS_BINDIR
#elif defined(__linux)
    #define NEEDS_UNISTD
    #define READ_PATH_FROM_PROC_SELF_EXE
#elif defined(__APPLE__)
    #define READ_PATH_FROM_ARGV0
    #define HAS_RESOURCES_PREFIX
#else
    #define READ_PATH_FROM_ARGV0
#endif
   
#ifdef NEEDS_UNISTD
#include <unistd.h>
#endif



#define RAND ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)

//#define ENABLE_PROFILE
#include <profile.h>

using namespace std;

Game * Game::the_game = 0;


void setup_paths(Ptr<IConfig> config, const char **argv) {
#if defined(READ_PATH_FROM_ARGV0)
    std::string bin = argv[0];
#elif defined(READ_PATH_FROM_PROC_SELF_EXE)
    std::string bin;
    {
        char buf[256];
        ssize_t length = readlink("/proc/self/exe", buf, 256);
        if (-1 == length || 256 == length) {
            throw runtime_error("Could not open /proc/self/exe to establish binary location.");
        }
        buf[length] = 0; // append 0 byte to buffer
        bin = buf;
    }
#endif

    int last_sep = bin.find_last_of("/\\");
    if (last_sep == std::string::npos) {
        ls_error("Strange binary location without separators: %s\n", bin.c_str());
        throw runtime_error("Failed to determine binary location");
    }
    std::string bin_dir = bin.substr(0,last_sep);
    
#ifdef PREFIX_IS_BINDIR
    std::string prefix = bin_dir;
#else
    last_sep = bin_dir.find_last_of("/\\");
    if (last_sep == std::string::npos) {
        ls_error("Binary location should be something like /usr/bin but is: %s\n", bin_dir.c_str());
        throw runtime_error("Failed to determine binary location");
    }
    std::string prefix = bin_dir.substr(0, last_sep);
#endif
    
    config->set("base_dir",prefix.c_str());
    config->set("bin_dir",bin_dir.c_str());
    
#ifdef HAS_RESOURCES_PREFIX
    std::string resources_dir = prefix + "/Resources";
#else
    std::string resources_dir = prefix;
#endif
    config->set("resources_dir", resources_dir.c_str());
    
    std::string data_dir = resources_dir + "/share/tnl";
    config->set("data_dir",(data_dir).c_str());
    config->set("Io_system_init_script_1",(data_dir + "/scripts/system_init_1.io").c_str());
}


Game::Game(int argc, const char **argv)
: argc(argc), argv(argv)
, debug_mode(false)
, mouse_grabbed(false)
, debug_data(new DataNode)
, view_is_external(false)
, render_context(0)
{
    the_game = this;
}

Game::~Game()
{
    the_game = 0;
}

void Game::startupSystem(Status & stat) {
    stat.beginJob("System startup", 1);

    ls_message("Initializing configuration system:\n");
    config= new Config;

    // Set some default locations based on the binary location given in argv[0]
    setup_paths(config, argv);

    // Pass command line arguments to configuration, if any
    // The locations given above may be overridden here
    config->feedArguments(argc, argv);
    
    ls_message("Creating IoScriptingManager (system)\n");
    Ptr<IoScriptingManager> system_scripting_manager = new IoScriptingManager(this);
    ls_message("Adding basic mappings...");
    addBasicMappings(this, system_scripting_manager->getMainState());
    ls_message("done");
    {
        char buf[256];
		strncpy(buf,config->query("Io_system_init_script_1", "<noinit>"),256);
        if (0==strcmp(buf,"<noinit>")) {
            throw runtime_error("Io_system_init_script_1 not found.");
        }
        ls_message("Executing Io script \"%s\"\n", buf);
		IoState_doFile_(system_scripting_manager->getMainState(), buf);
    }
    ls_message("Back in C++.\n");

    event_remapper = new EventRemapper();
    event_remapper->sig_action_triggered.connect(SigC::slot(*this, &Game::actionTriggered));

    ls_message("Initializing SDL: ");
    if (-1 == SDL_Init( SDL_INIT_VIDEO |
                        SDL_INIT_JOYSTICK |
                        (config->queryBool("Game_enable_SDL_parachute", true)?
                        	0:SDL_INIT_NOPARACHUTE) )  )
    {
        const char * err = SDL_GetError();
        ls_error("error: %s\n", err);
        throw runtime_error(err);
    }
    SDL_EnableUNICODE(true);
    SDL_EnableKeyRepeat(0, 0);

    ls_message("Found %d joysticks.\n", SDL_NumJoysticks());
    SDL_JoystickEventState(SDL_ENABLE);
    for(int i=0; i<SDL_NumJoysticks(); i++) {
        SDL_JoystickOpen(i);
        ls_message("  Joystick %d: '%s'\n", i, SDL_JoystickName(i));
    }
    ls_message("done.\n");

    ls_message("Initializing video.\n");
    {
        int xres = config->queryInt("Game_xres", 1024);
        int yres = config->queryInt("Game_yres", 768);
        bool fullscreen = config->queryBool("Game_fullscreen", false);
        bool autores = config->queryBool("Game_auto_resolution", false);
        
        if (fullscreen) {
            const SDL_VideoInfo * info = SDL_GetVideoInfo();
            
            char buf[16];
            
            if (autores) {
                xres = info->current_w;
                yres = info->current_h;
                sprintf(buf, "%d", xres);
                config->set("Game_xres", buf);
                sprintf(buf, "%d", yres);
                config->set("Game_yres", buf);
            } else {
                config->set("Game_restore_resolution", "true");
                sprintf(buf, "%d", xres);
                config->set("Game_restore_resx", buf);
                sprintf(buf, "%d", yres);
                config->set("Game_restore_resy", buf);
            }
        }
        
        ls_message("Requested mode: %dx%d (%s, %s)\n",
                xres, yres,
                fullscreen?"fullscreen":"in a window",
                autores?"auto-detected":"manual");

        SDL_GL_SetAttribute( SDL_GL_RED_SIZE, config->queryInt("Game_red_bits", 5) );
        SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, config->queryInt("Game_green_bits", 5) );
        SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, config->queryInt("Game_blue_bits", 5) );
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, config->queryInt("Game_zbuffer_bits", 16) );
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
        if (config->queryBool("Game_fsaa_enabled", false)) {
            SDL_GL_SetAttribute (SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute (SDL_GL_MULTISAMPLESAMPLES,
                config->queryInt("Game_fsaa_samples", 4));
        }
#ifndef NDEBUG
        if (fullscreen) {
            fullscreen = false;
            ls_message("Fullscreen mode disabled: This is a debug build.\n");
        }
#endif
        if(fullscreen) {
            surface = SDL_SetVideoMode(xres, yres, 32,
                    SDL_OPENGL | SDL_FULLSCREEN);
        } else {
            surface = SDL_SetVideoMode(xres, yres, 32, SDL_OPENGL);
        }
        if (!surface) {
            ls_error("Failed requesting video mode.\n");
            throw runtime_error("Could not initialize OpenGL surface.");
        }
        int r=0,g=0,b=0,d=0,db=0;
        SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &r );
        SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &g );
        SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &b );
        SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &d );
        SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &db );
        ls_message(" got r/g/b/d %d/%d/%d/%d %s double buffering. ",
                r,g,b,d,db?"with":"without");
        ls_message("Initializing OpenGL renderer.\n");
        renderer = new JOpenGLRenderer();
        renderer->resize(xres, yres);
        ls_message("Done initializing OpenGL renderer.\n");
        
        SDL_WM_SetCaption("Thunder&Lightning http://tnlgame.net/", "Thunder&Lightning");
    }
    ls_message("Done initializing video.\n");
    
    ls_message("Performing second stage of Io initialization.\n");
    {
		char buf[256];
		strncpy(buf,config->query("Io_system_init_script_2","<noinit>"),256);
        if (0==strcmp(buf,"<noinit>")) {
            throw runtime_error("Io_system_init_script_2 not found.");
        }
		IoState_doFile_(system_scripting_manager->getMainState(), buf);
    }
    ls_message("Done.\n");
    
    ls_message("Initializing GLEW:\n");
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        ls_error("Error: %s\n", glewGetErrorString(err));
        throw runtime_error("Could not initialize GLEW library.");
    }
    if (GLEW_VERSION_1_3) ls_message("  - detected OpenGL 1.3 support\n");
    if (GLEW_VERSION_1_4) ls_message("  - detected OpenGL 1.4 support\n");
    if (GLEW_VERSION_1_5) ls_message("  - detected OpenGL 1.5 support\n");
    if (GLEW_VERSION_2_0) ls_message("  - detected OpenGL 2.0 support. Nice!\n");
    ls_message("Done.\n");
    
    ls_message("Initializing managers... ");
    texman = new TextureManager(*config, *renderer);
    modelman = new ModelMan(texman);
    fontman = new FontMan(this);
    soundman = new ALSoundMan(config);
    ls_message("Done initializing managers\n");

    ls_message("Initializing CEGUI library.\n");
    try {
        using namespace CEGUI;
        
        CEGUI::Logger * logger = new TnlCeguiLogger;

        CEGUI::DefaultResourceProvider * resource_provider = new CEGUI::DefaultResourceProvider;
        resource_provider->setResourceGroupDirectory("", config->query("Game_cegui_dir","."));
        
        resource_provider->setResourceGroupDirectory("schemes", config->query("Game_cegui_schemes_dir","."));
        Scheme::setDefaultResourceGroup("schemes");

        resource_provider->setResourceGroupDirectory("imagesets", config->query("Game_cegui_imagesets_dir","."));
        Imageset::setDefaultResourceGroup("imagesets");

        resource_provider->setResourceGroupDirectory("fonts", config->query("Game_cegui_fonts_dir","."));
        Font::setDefaultResourceGroup("fonts");

        resource_provider->setResourceGroupDirectory("layouts", config->query("Game_cegui_layouts_dir","."));
        WindowManager::setDefaultResourceGroup("layouts");

        resource_provider->setResourceGroupDirectory("looknfeels", config->query("Game_cegui_looknfeels_dir","."));
        WidgetLookManager::setDefaultResourceGroup("looknfeels");

        CEGUI::OpenGLRenderer & opengl_renderer = CEGUI::OpenGLRenderer::create();

        CEGUI::System::create(opengl_renderer, resource_provider);
        
        // load in the scheme file, which auto-loads the TaharezLook imageset
        CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");

        // load in a font.  The first font loaded automatically becomes the default font.
        CEGUI::FontManager::getSingleton().create("Commonwealth-10.font");
        
        CEGUI::System::getSingleton().setDefaultMouseCursor("TaharezLook", "MouseArrow");
        
        main_gui = new UI::MainGUI(this);
    } catch (CEGUI::Exception & ex) {
        ls_error("CEGUI error: %s\n", ex.getMessage().c_str());
        throw;
    }
    ls_message("Done.\n");

    stat.endJob();
}

void Game::teardownSystem(Status & stat) {
    stat.beginJob("System teardown");
    
    event_remapper = 0;

    texman->shutdown();
    texman = 0;

    fontman = 0;
	soundman->shutdown();
    soundman = 0;
    modelman = 0;
    
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(SDL_ENABLE);
    
    if (config->queryBool("Game_restore_resolution", false)) {
        ls_message("Restoring screen resolution.\n");
        SDL_SetVideoMode(
            config->queryInt("Game_restore_resx"),
            config->queryInt("Game_restore_resy"),
            32, SDL_FULLSCREEN);
    }
    
    ls_message("Exiting SDL.\n");
    SDL_Quit();
    ls_message("Exiting game.\n");
    
    stat.endJob();
}

void Game::startupSimulation(Status & stat) {
    stat.beginJob("Simulation startup", 12);
    
    stat.beginJob("Initializing CollisionManager");
    collisionman = new Collide::CollisionManager();
    stat.nextJob("Initializing clock");
    clock = new Clock;
   	stat.nextJob("Initializing Environment");
    environment = new Environment(this);
    stat.nextJob("Initializing Water");
    water = new Water(this);
   	stat.nextJob("Initializing Camera");
   	camera = new Camera(this);
   	{
        JCamera jcamera;
        camera->getCamera(&jcamera);
        renderer->setCamera(&jcamera.cam);
    }
    stat.nextJob("Initialize LOD terrain",1);
    quadman = new LoDQuadManager(this, stat);

    stat.nextJob("Initializing SkyBox");
    skybox = new SkyBox(this);
    stat.nextJob("Initializing controls");
    event_sheet = new EventSheet();
    event_remapper->addEventSheet(event_sheet);
    initControls();

    // setup some default rendering until a script initializes something else
    stat.nextJob("Initializing main render pass");
    RenderContext ctx(camera);
    renderpass_main = new SceneRenderPass(this, ctx);

    // The usual overlay consists of info messages and debug info
    stat.nextJob("Initializing overlay pass");
    Ptr<FlexibleGunsight> overlay = new FlexibleGunsight(this);
    overlay->addStaticInfoMessage(this);
    overlay->addStaticDebugInfo(this);
    overlay->addProfilingGraph(this);
    Ptr<UI::PanelRenderPass> overlay_pass = new UI::PanelRenderPass(renderer);
    overlay_pass->setPanel(overlay);
    this->renderpass_overlay = overlay_pass;
    overlay_pass->stackedOn(renderpass_main);
    
    // Create the Io scripting manager responsible for high-level manipulation
    // and low-level AI during this simulation session.
    stat.nextJob("Initializing Io scripting manager (simulation)");
    io_scripting_manager = new IoScriptingManager(this);
    addBasicMappings(this, io_scripting_manager->getMainState());
    addMappings(this, io_scripting_manager->getMainState());
    ls_message("Performing Io simulation initialization.\n");
    {
		char buf[256];
		strncpy(buf,config->query("Io_simulation_init_script","<noinit>"),256);
        if (0==strcmp(buf,"<noinit>")) {
            throw runtime_error("Io_simulation_init_script not found.");
        }
		IoState_doFile_(io_scripting_manager->getMainState(), buf);
    }
    ls_message("Done.\n");

    stat.nextJob("Initializing Console");
    console = new UI::Console(this);
    stat.endJob();
    
    // Every simulation session is marked non-interactive initially
    setInteractive(false);

    stat.endJob();
}

void Game::teardownSimulation(Status & stat) {
    stat.beginJob("Simulation teardown",5);
    stat.beginJob("Clearing event remapper");
    event_remapper->removeEventSheet(event_sheet);
    event_sheet = 0;
    stat.nextJob("Removing Io scripting manager (simulation)");
    io_scripting_manager = 0;
    stat.nextJob("Removing actors from scene");
    current_actor = 0;
    removeAllActors();
    stat.nextJob("Cleaning up");
    current_view = 0;
    previous_view = 0;
    clock = 0;
    camera = 0;
    quadman = 0;
    skybox = 0;
    environment = 0;
    water = 0;
    console = 0;
    renderpass_main = 0;
    renderpass_overlay = 0;
    stat.nextJob("Removing collision manager");
    collisionman = 0;
    stat.endJob();
    stat.endJob();
}

void Game::run()
{
	Ptr<IGame> guard = this;
    game_done=false;
    
    {
        Status stat;
        startupSystem(stat);
        LoadingScreen lscr(this, config->query("Game_loading_screen"));
        stat.getSignal().connect(SigC::slot(lscr, &LoadingScreen::update));
        startupSimulation(stat);
    }
    
    // Start the demo script
    IoState_doFile_(io_scripting_manager->getMainState(), config->query("Game_demo_script"));
    // Show main menu
    main_gui->switchToMainMenu(false, false);

    doEvents();
    while (!game_done) {
        IoState_pushRetainPool(io_scripting_manager->getMainState());
        doFrame();
        IoState_popRetainPool(io_scripting_manager->getMainState());
    }
    
    {
        Status stat;
        {
            LoadingScreen lscr(this, config->query("Game_loading_screen"));
            stat.getSignal().connect(SigC::slot(lscr, &LoadingScreen::update));
            teardownSimulation(stat);
        }
        teardownSystem(stat);
    }
}


Ptr<TextureManager> Game::getTexMan() {
    return texman;
}

JRenderer *Game::getRenderer()
{
    return renderer;
}

Ptr<EventRemapper> Game::getEventRemapper()
{
    return event_remapper;
}

Ptr<IModelMan> Game::getModelMan()
{
    return modelman;
}

Ptr<Collide::CollisionManager> Game::getCollisionMan()
{
    return collisionman;
}


Ptr<IConfig> Game::getConfig()
{
    return config;
}


Ptr<ICamera> Game::getCamera()
{
    return camera;
}

UI::Surface Game::getScreenSurface() {
    int xres = renderer->getWidth();
    int yres = renderer->getHeight();
    float aspect = renderer->getAspect();
    float focus = renderer->getFocus();
    float dist = (renderer->getClipNear() + renderer->getClipFar())/2;
    return UI::Surface::FromCamera(aspect, focus, xres, yres, dist);
}

Ptr<Clock> Game::getClock()
{
    return clock;
}

Ptr<ITerrain> Game::getTerrain()
{
    return quadman;
}

Ptr<IFontMan> Game::getFontMan() {
    return fontman;
}

Ptr<SoundMan> Game::getSoundMan() {
    return soundman;
}

Ptr<Environment> Game::getEnvironment() {
    return environment;
}

Ptr<Water> Game::getWater() {
    return water;
}

Ptr<IoScriptingManager> Game::getIoScriptingManager() {
	return io_scripting_manager;
}

void Game::infoMessage(const char * msg, const Vector color) {
    info_message_signal(msg,color);
}

double Game::getTimeDelta()
{
    return clock->getStepDelta() * 1000.0;
}

Ptr<IView> Game::getCurrentView()
{
	return current_view;
}

void Game::setCurrentView(Ptr<IView> view)
{
    if (view) {
        view->enable();
        if (view->getRenderPass()) {
            renderpass_main = view->getRenderPass();
            renderpass_overlay->stackedOn(renderpass_main);
        } else {
            RenderContext ctx(camera);
            renderpass_main = new SceneRenderPass(this, ctx);
            renderpass_overlay->stackedOn(renderpass_main);
        }
    } else {
        RenderContext ctx(camera);
        renderpass_main = new SceneRenderPass(this, ctx);
        renderpass_overlay->stackedOn(renderpass_main);
    }
    
    if (current_view) {
        current_view->disable();
    }
    current_view = view;
    view_is_external = false;
}

Ptr<IActor> Game::getCurrentlyControlledActor()
{
	return current_actor;
}

void Game::setCurrentlyControlledActor(Ptr<IActor> actor)
{
	if (current_actor) {
		if (current_actor->hasControlMode(IActor::AUTOMATIC)) {
			current_actor->setControlMode(IActor::AUTOMATIC);
		} else {
			current_actor->setControlMode(IActor::UNCONTROLLED);
		}
	}
	
	current_actor = actor;
    if (!actor) return;
	if (current_actor->hasControlMode(IActor::MANUAL)) {
		current_actor->setControlMode(IActor::MANUAL);
	}
}

bool Game::debugMode() {
    return debug_mode;
}

Ptr<DataNode> Game::getDebugData() {
    return debug_data;
}


void Game::doEvents()
{
    SDL_Event event;
    
    event_remapper->beginEvents();
    while(SDL_PollEvent(&event)) { // Loop while there are events on the queue
        // special-case handling for some SDL events
        
        // If we don't have mouse focus, the first click grabs it again
        if (event.type == SDL_MOUSEBUTTONDOWN && !mouse_grabbed) {
            mouse_grabbed = true;
            
            if (config->queryBool("Game_grab_mouse",false)) {
                SDL_WM_GrabInput(SDL_GRAB_ON);
                SDL_ShowCursor(SDL_DISABLE);
            }
            ls_message("Mouse grab aquired.\n");
        }
        
        if (event.type == SDL_ACTIVEEVENT) {
            ls_message("ActiveEvent of state %d with gain %d\n", event.active.state, event.active.gain);
            if (!event.active.gain) {
                SDL_WM_GrabInput(SDL_GRAB_OFF);
                SDL_ShowCursor(SDL_ENABLE);
                mouse_grabbed = false;
                ls_message("Mouse grab released.\n");
            }
            if ((event.active.state & SDL_APPACTIVE) && event.active.gain==0) {
                ls_message("Game inactive.\n");
                while(SDL_WaitEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        ls_message("Game quitting.\n");
                        break;
                    } else if (event.type == SDL_ACTIVEEVENT && (event.active.state&SDL_APPACTIVE) && event.active.gain) {
                        ls_message("Game active again.\n");
                        break;
                    }
                }
            }
        }
        
        if (event.type == SDL_QUIT) {
            endGame();
        } else {
            event_remapper->feedEvent(event);
        }
    }
    event_remapper->endEvents();
}


void Game::setupRenderer()
{
    Vector col = environment->getFogColor();
    col *= 256.0;
    jcolor3_t fog_col;
    fog_col.r = col[0];
    fog_col.g = col[1];
    fog_col.b = col[2];
    renderer->setFogColor(&fog_col);
    renderer->setFogType(JR_FOGTYPE_LINEAR,0.0f);
    renderer->enableFog();
}

void Game::updateSimulation()
{
    float MAX_STEP_DELTA = config->queryFloat("Game_max_step_delta", 1.0f/30);
    float MAX_FRAME_DELTA = config->queryFloat("Game_max_frame_delta", 1.0f/15);
    int MAX_MS_FOR_SIMULATION = config->queryInt("Game_max_ms_for_simulation", 1000/30);
    
    clock->update();
    int t0 = SDL_GetTicks();
    // this will return false if pause is activated
    while(clock->catchup(MAX_STEP_DELTA)) {
        collisionman->run(this, clock->getStepDelta());
        cleanupActors();
        setupActors();
        RadarNet::updateAllRadarNets(clock->getStepDelta());
        
        if (SDL_GetTicks() - t0 >= MAX_MS_FOR_SIMULATION) {
            break;
        }
        
        if (clock->getFrameDelta() >= MAX_FRAME_DELTA) {
            break;
        }
    }
    clock->skip();
}

void Game::updateView()
{
    // Upon death of current view subject, switch to external perspective
    if (current_view && !current_view->getViewSubject()->isAlive()) {
        externalView();
        event_remapper->triggerAction("current_view_subject_killed");
    }
}

void Game::updateSound() {
    soundman->update(clock->getFrameDelta());

    if (current_view) {
        Vector pos = current_view->getViewHead()->getLocation();
        soundman->setListenerPosition(pos);

        Vector up, right, front;
        current_view->getViewHead()->getOrientation(&up, &right, &front);
        soundman->setListenerOrientation(up, front);

        soundman->setListenerVelocity(current_view->getViewHead()->getMovementVector());
        soundman->setListenerVelocity(Vector(0,0,0));
    }
}

void Game::setupMainRender() {
}

void Game::updateIoScripting() {
   // notifiy Io scripting that another frame has passed by sending a
    // "postFrame" message to Game
    if (!clock->isPaused()) {
        IoObject* self = getProtoObject<Ptr<IGame> >(
            io_scripting_manager->getMainState());

        if (IoObject_rawGetSlot_(self, IOSYMBOL("postFrame"))) {
            IoMessage *msg =
                IoMessage_newWithName_label_(IOSTATE, IOSYMBOL("postFrame"),IOSYMBOL("Game::postFrame"));
            IoState_tryToPerform(IOSTATE, self, IOSTATE->lobby, msg);
        }
    }
}

Ptr<SceneRenderPass> Game::createRenderPass(Ptr<IMovementProvider> view) {
    Ptr<FollowingCamera> camera = new FollowingCamera;
    camera->setTarget(view);
    camera->setFocus(config->queryFloat("Camera_focus", 1.5));
    camera->setAspect(config->queryFloat("Camera_aspect", 1));
    camera->setNearDistance(environment->getClipMin());
    camera->setFarDistance(environment->getClipMax());
    
    RenderContext ctx(camera);
    return new SceneRenderPass(this, ctx);
}

void Game::renderScene(SceneRenderPass * pass)
{
    Uint32 t0,t1;
    
    RenderContext* ctx = &pass->context;
    this->render_context = ctx;
    
    Ptr<ICamera> old_camera = getCamera();
    this->camera = ctx->camera;
    
    JCamera jcamera;
    this->camera->getCamera(&jcamera);
    renderer->setCamera(&jcamera.cam);
    renderer->setClipRange(camera->getNearDistance(),
                           camera->getFarDistance());
    
    environment->update(this->camera);
    
    Ptr<JDirectionalLight> sun = renderer->createDirectionalLight();
    sun->setColor(Vector(1,1,1) - 0.25*Vector(.97,.83,.74));
    sun->setEnabled(true);
    sun->setDirection(Vector(-0.9, 0.4, 0).normalize());
    
    if (ctx->draw_skybox) skybox->draw();

    t0 = SDL_GetTicks();
    if (ctx->clip_above_water) renderer->pushClipPlane(Vector(0,-1,0), 0);
    if (ctx->clip_below_water) renderer->pushClipPlane(Vector(0,1,0), 0);
    if (ctx->draw_terrain) quadman->draw();
    if (ctx->clip_above_water) renderer->popClipPlanes(1);
    if (ctx->clip_below_water) renderer->popClipPlanes(1);
    t1 = SDL_GetTicks();
    getDebugData()->setInt("render_terrain", (int) (t1-t0));

    t0 = SDL_GetTicks();
    if (ctx->draw_water) {
        renderer->setZBufferFunc(JR_ZBFUNC_LESS);
        water->draw(pass);
        renderer->setZBufferFunc(JR_ZBFUNC_LEQUAL);
    }
    t1 = SDL_GetTicks();
    getDebugData()->setInt("render_water", (int) (t1-t0));

    t0 = SDL_GetTicks();
    if (ctx->clip_above_water) renderer->pushClipPlane(Vector(0,-1,0), 0);
    if (ctx->clip_below_water) renderer->pushClipPlane(Vector(0,1,0), 0);
    if (ctx->draw_actors) drawActors();
    if (ctx->clip_above_water) renderer->popClipPlanes(1);
    if (ctx->clip_below_water) renderer->popClipPlanes(1);
    t1 = SDL_GetTicks();
    getDebugData()->setInt("render_actors", (int) (t1-t0));

    this->camera = old_camera;
    
    this->render_context = 0;
}

void Game::doFrame()
{
    Uint32 t[16];
    int n=0;
    t[n++] = SDL_GetTicks(); // mainloop_0:
    doEvents();
    t[n++] = SDL_GetTicks(); // mainloop_1:
    updateSimulation();
    t[n++] = SDL_GetTicks(); // mainloop_2:
    updateView();
    t[n++] = SDL_GetTicks(); // mainloop_3:
    updateSound();
    t[n++] = SDL_GetTicks(); // mainloop_4:

    setupRenderer();
    t[n++] = SDL_GetTicks(); // mainloop_5:
    setupMainRender();
    t[n++] = SDL_GetTicks(); // mainloop_6:
    water->update();
    t[n++] = SDL_GetTicks(); // mainloop_7:

    pre_draw.emit();
    
    // draw all layers: renderpass, optional texture mosaic, GUI, console
    renderpass_overlay->render();
    if (debug_mode) renderpass_overlay->drawMosaic();
    CEGUI::System::getSingleton().renderGUI();
    console->draw(renderer);
    
    post_draw.emit();
    
    t[n++] = SDL_GetTicks(); // mainloop_8:
    SDL_GL_SwapBuffers();
    t[n++] = SDL_GetTicks(); // mainloop_9:

    updateIoScripting();
    t[n++] = SDL_GetTicks();
    
    // Put the measured times into the debug data interface
    Uint32 sum=0;
    while (--n>0) {
        char buf[32];
        sprintf(buf, "mainloop_%d", n-1 );
        getDebugData()->setInt(buf, (int) (t[n]-t[n-1]));
        sum += t[n]-t[n-1];
    }
    getDebugData()->setInt("mainloop_sum", (int) sum);
}

const RenderContext *Game::getCurrentContext()
{
    return render_context;
}

void Game::restartSimulation() {
    Status stat;
    LoadingScreen lscr(this, config->query("Game_loading_screen"));
    stat.getSignal().connect(SigC::slot(lscr, &LoadingScreen::update));
    stat.beginJob("Restarting simulation", 2);
    teardownSimulation(stat);
    startupSimulation(stat);
    stat.endJob();
}

void Game::clearScreen() {
    SDL_GL_SwapBuffers();
    renderer->clear(true, true);
}

void Game::togglePauseMode() {
    if (clock->isPaused()) clock->resume(); else clock->pause();
}

void Game::toggleDebugMode() {
    debug_mode = !debug_mode;
}

void Game::mainMenu() {
    if (UI::MainGUI::OFF == main_gui->currentState()) {
        main_gui->switchToMainMenu(true, true);
    }
}

void Game::endGame() {
    game_done=true;
}

bool Game::isInteractive() {
    return is_interactive;
}

void Game::setInteractive(bool b) {
    is_interactive = b;
}

void Game::accelerateSpeed() {
    clock->setTimeFactor(clock->getTimeFactor() * 1.5);
}

void Game::decelerateSpeed() {
    clock->setTimeFactor(clock->getTimeFactor() / 1.5);
}

void Game::setView(int n) {
	if (!current_view || !current_view->getViewSubject()) return;
	if (n >= current_view->getViewSubject()->getNumViews()) return;
	setCurrentView(current_view->getViewSubject()->getView(n));
}

void Game::externalView() {
	if (view_is_external) {
	    if (previous_view) {
            setCurrentView(previous_view);
            view_is_external = false;
            
            // After switching back to a dead actor, immediately switch to the next.
            // Otherwise, release control from whatever the previous actor was but
            // don't take over control yet.
            if ( ! current_view->getViewSubject()->isAlive()) {
                nextTarget();
            } else {
                setCurrentlyControlledActor(0);
            }
        }
    } else {
        previous_view = current_view;
        
        Ptr<Observer> observer = new Observer(this);
        if (current_view) {
            observer->setLocation(current_view->getViewHead()->getLocation());
            Vector up, right, front;
            current_view->getViewHead()->getOrientation(&up, &right, &front);
            observer->setOrientation(up, right, front);
        }
        addWeakActor(ptr(observer));
		setCurrentView(observer->getView(0));
		if (!debugMode()) setCurrentlyControlledActor(observer);
		view_is_external = true;
	}
}

void Game::nextTarget() {
    typedef const std::vector<Ptr<IActor> > List;
    typedef List::const_iterator Iter;
    
    // In case of external view, this is equivalent to switching back to internal
    if (view_is_external) {
        externalView();
        return;
    }
    
    Ptr<Faction> current_faction = getCurrentView()->getViewSubject()->getFaction();
    
    List & list = actors;
    Iter current = list.begin();
    if(current_view && current_view->getViewSubject()) {
    	current = find(list.begin(),list.end(), 
    		current_view->getViewSubject());
    }
    if (current == list.end()) current = list.begin();
    if (current==list.end()) {
        // If the list is completely empty, switch to external view
        externalView();
        return;
    }
    
    Iter next = current;
    if (debugMode()) {
    	++next;
        if (next == list.end()) next = list.begin();
    } else {
        ++next;
        if (next == list.end()) next = list.begin();
        while (next != current) {
            if ((*next)->hasControlMode(IActor::MANUAL) &&
                (*next)->getFaction() == current_faction)
            {
                break;
    		}
    		++next;
            if (next == list.end()) next = list.begin();
    	}
    }
    if (next == current) {
        // If we couldn't find any other actor, do nothing.
        return;
    }
    
    setCurrentView((*next)->getView(0));
    
    // Make sure no actor is under manual control now
    setCurrentlyControlledActor(0);
}

void Game::toggleControlMode() {
	if (!current_view || !current_view->getViewSubject()) {
		current_actor = 0;
		return;
	}
    if (current_actor && current_actor == current_view->getViewSubject()) {
        setCurrentlyControlledActor(0);
    } else {
        setCurrentlyControlledActor(current_view->getViewSubject());
    }
}

void Game::actionTriggered(const char *action) {
    IoObject* self = getProtoObject<Ptr<IGame> >(
        io_scripting_manager->getMainState());

    IoMessage *message = IoMessage_newWithName_label_(IOSTATE, IOSYMBOL("onMessage"),
        IOSYMBOL("Game::actionTriggered"));
    
    IoMessage_addCachedArg_(message, IOSYMBOL(action));
    IoMessage_addCachedArg_(message, IONIL(self));

    
    IoObject * result = IoState_tryToPerform(IOSTATE, self, IOSTATE->lobby, message);
}

int main(int argc, char *argv[])
{
    try {
        Ptr<Game> game = new Game(argc, (const char**)argv);
        game->run();
        if (game->getRefs() > 1) {
            ls_warning("Game still has %d references. Killing Game.\n", game->getRefs());
            Object::debug();
            Game * g = ptr(game);
            game = 0;
            delete g;
        }
    } catch (std::exception & e) {
        ls_error("Caught exception: %s\n", e.what());
#ifndef NDEBUG
        // Pass on the exception to the debugger
        throw;
#endif
    }
    return 0;
}

