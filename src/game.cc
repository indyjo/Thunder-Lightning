#include <cstdio>
#include <cstdlib>
#include <list>
#include <algorithm>
#include <sigc++/bind.h>
#include <modules/math/Vector.h>
#include <modules/camera/camera.h>
#include <modules/clock/clock.h>
#include <modules/LoDTerrain/LoDTerrain.h>
#include <modules/skybox/skybox.h>
#include <modules/map/map.h>
#include <modules/actors/drone/drone.h>
#include <modules/actors/projectiles/bullet.h>
#include <modules/actors/projectiles/dumbmissile.h>
#include <modules/actors/projectiles/smartmissile.h>
#include <modules/actors/tank/tank.h>
#include <modules/environment/environment.h>
#include <modules/model/model.h>
#include <modules/model/modelman.h>
#include <modules/fontman/fontman.h>
#include <modules/ui/loadingscreen.h>
#include <modules/collide/CollisionManager.h>
#include <modules/ui/Console.h>
#include <modules/ui/Surface.h>
#include <modules/scripting/IoScriptingManager.h>
#include <modules/scripting/mappings.h>
#include <modules/actors/RigidActor.h>
#include <modules/actors/Observer.h>
#include <modules/config/config.h>


#include <sound.h>
#include <Faction.h>
#include "game.h"

#include <stdexcept>

#define RAND ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)

using namespace std;

Game * Game::the_game = 0;

Game::Game(int argc, const char **argv)
: argc(argc), argv(argv)
, debug_mode(false)
, view_is_external(false)
{
    the_game = this;

    ls_message("Initializing configuration system:\n");
    config= new Config;

    // Set some default locations based on the binary location given in argv[0]
    string exe = argv[0];
    volatile int last_sep = exe.find_last_of("/\\");
    string exe_dir = exe.substr(0,last_sep);
    last_sep = exe_dir.find_last_of("/\\");
    string prefix = exe_dir.substr(0,last_sep);

    config->set("base_dir",prefix.c_str());
    config->set("bin_dir",exe_dir.c_str());
    config->set("Io_init_script",(prefix + "/share/landscape/scripts/init.io").c_str());

    // Pass command line arguments to configuration, if any
    // The locations given above may be overridden here
    config->feedArguments(argc, argv);
    
    ls_message("Continuing initialization from Io language:\n");
    io_scripting_manager = new IoScriptingManager(this);
    addBasicMappings(this, io_scripting_manager->getMainState());
    {
        char buf[256];
		strncpy(buf,config->query("Io_init_script", "<noinit>"),256);
        if (0==strcmp(buf,"<noinit>")) {
            throw runtime_error("Io_init_script not found.");
        }
		IoState_doFile_(io_scripting_manager->getMainState(), buf);
    }
    ls_message("done.\n");

    event_remapper = new EventRemapper();

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
    }
    ls_message("done.\n");

    ls_message("Initializing video.\n");
    {
        int xres = config->queryInt("Game_xres", 1024);
        int yres = config->queryInt("Game_yres", 768);
        bool fullscreen = config->queryBool("Game_fullscreen", false);

        ls_message("Requested mode: %dx%d %s\n",
                xres, yres,
                fullscreen?"(fullscreen)":"(in a window)");

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
                r,g,b,d,db?"width":"without");
        ls_message("Initializing OpenGL renderer.\n");
        renderer = new JOpenGLRenderer(xres, yres, config->queryFloat("Camera_aspect",1.33333));
        ls_message("Done initializing OpenGL renderer.\n");
    	camera = new Camera(this);
    	JCamera jcam;
    	camera->getCamera(&jcam);
    	renderer->setCamera(&jcam.cam);
    	renderer->setClipRange(0.1, 10);
    }
    ls_message("Done initializing video.\n");

    if (config->queryBool("Game_grab_mouse",false)) {
	    SDL_WM_GrabInput(SDL_GRAB_ON);
	    SDL_ShowCursor(SDL_DISABLE);
    }

    ls_message("Initializing managers... ");
    texman = new TextureManager(*renderer);
    modelman = new ModelMan(texman);
    fontman = new FontMan(this);
    soundman = new SoundMan(config);
    collisionman = new Collide::CollisionManager();
    clock = new Clock;
    ls_message("Done initializing managers\n");

    {
    	ls_message("Querying from config %p:\n", ptr(config));
        string background = config->query("Game_loading_screen");
        ls_message("Initializing loading screen [%s]:\n", background.c_str());
        LoadingScreen lscr(this, background);
        ls_message("done.\n");
        Status stat;
        stat.getSignal().connect(SigC::slot(lscr, &LoadingScreen::update));

        stat.beginJob("Initializing", 2);
        initModules(stat);
        stat.stepFinished();
        ls_message("Initializing controls ...");
        initControls();
        ls_message("done\n");
        stat.endJob();
    }
    
    console = new UI::Console(this, getScreenSurface());
    addMappings(this, io_scripting_manager->getMainState());
    {
		char buf[256];
		strncpy(buf,config->query("Io_init_script_2","<noinit>"),256);
        if (0==strcmp(buf,"<noinit>")) {
            throw runtime_error("Io_init_script_2 not found.");
        }
		ls_message("Executing initial setup script: %s\n", buf);
		IoState_doFile_(io_scripting_manager->getMainState(), buf);
		ls_message("Done excuting initial setup script: %s\n", buf);
    }

    while (clock->catchup(1.0f)) ;
}

Game::~Game()
{
    the_game = 0;

    event_remapper = 0;
    ls_message("Exiting SDL.\n");
    SDL_Quit();
    ls_message("Exiting game.\n");
}

void Game::run()
{
	Ptr<IGame> guard = this;
    game_done=false;

    Ptr<JDirectionalLight> sun = renderer->createDirectionalLight();
    sun->setColor(Vector(1,1,1) - 0.25*Vector(.97,.83,.74));
    sun->setEnabled(true);
    
    doEvents();
    while (!game_done) {
        //int t0 = SDL_GetTicks();
        IoState_pushRetainPool(io_scripting_manager->getMainState());
        //int t1 = SDL_GetTicks();
        preFrame();
        //int t2 = SDL_GetTicks();
    	sun->setDirection(Vector(-0.9, 0.4, 0).normalize());
        doFrame();
        //int t3 = SDL_GetTicks();
        postFrame();
        //int t4 = SDL_GetTicks();
        IoState_popRetainPool(io_scripting_manager->getMainState());
        //int t5 = SDL_GetTicks();
        //ls_message("Ticks: %d %d %d %d %d\n", t1-t0, t2-t1, t3-t2, t4-t3, t5-t4);
    }
    
    //Object::debug();
    
    removeAllActors();
    current_view = 0;
    previous_view = 0;
    current_actor = 0;
    texman->shutdown();
    texman = 0;

    event_remapper = 0;

    camera = 0;
    clock = 0;
    config = 0;
    fontman = 0;
	soundman->shutdown();
    soundman = 0;
    modelman = 0;
    collisionman = 0;
#if ENABLE_LOD_TERRAIN
    quadman = 0;
#endif
#if ENABLE_SKYBOX
    skybox = 0;
#endif
#if ENABLE_MAP
    map = 0;
#endif
#if ENABLE_GUNSIGHT
    gunsight = 0;
#endif
    io_scripting_manager = 0;
    ls_message("exiting with %d references left.\n", getRefs());
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(SDL_ENABLE);
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
    float xres = config->queryFloat("Game_xres",1024.0f);
    float yres = config->queryFloat("Game_yres",768.0f);
    float aspect = camera->getAspect();
    float focus = camera->getFocus();
    return UI::Surface::FromCamera(aspect, focus, xres, yres);
}

Ptr<Clock> Game::getClock()
{
    return clock;
}

Ptr<ITerrain> Game::getTerrain()
{
#if ENABLE_LOD_TERRAIN
    return quadman;
#endif
}

Ptr<IDrawable> Game::getGunsight()
{
    return gunsight;
}

void Game::setGunsight(Ptr<IDrawable> gunsight) {
    this->gunsight=gunsight;
}

#if ENABLE_SKY
Ptr<ISky> Game::getSky() {
    return sky;
}
#endif

Ptr<IFontMan> Game::getFontMan() {
    return fontman;
}

Ptr<SoundMan> Game::getSoundMan() {
    return soundman;
}

Ptr<Environment> Game::getEnvironment() {
    return environment;
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
        setGunsight(view->getGunsight());
    } else {
        setGunsight(0);
    }
    
    if (current_view) {
        current_view->disable();
    }
    current_view = view;
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


void Game::initModules(Status & stat)
{
    ls_message("initModules\n");
    int steps = 1;
#define STEPS 2
#if ENABLE_LOD_TERRAIN
    steps++;
#endif
#if ENABLE_SKYBOX
    steps++;
#endif
#if ENABLE_MAP
    steps++;
#endif

    stat.beginJob("Initialize modules", steps);

    environment = new Environment();
    stat.stepFinished();
#if ENABLE_LOD_TERRAIN
    stat.beginJob("Initialize LOD terrain",1);
    quadman = new LoDQuadManager(this, stat);
    stat.endJob();
#endif
#if ENABLE_SKYBOX
    ls_message("SkyBox init\n");
    skybox = new SkyBox(this);
    ls_message("end SkyBox init\n");
    stat.stepFinished();
#endif
#if ENABLE_MAP
    ls_message("map init\n");
    map = new Map(this);
    ls_message("end map init\n");
    stat.stepFinished();
#endif

    stat.endJob();
    ls_message("end LoDQuad::init\n");
}

namespace {
    template<class T> void setvar(T * x, const T v) { *x = v; }
}

void Game::initControls()
{
    Ptr<EventRemapper> r = getEventRemapper();
    r->sig_action_triggered.connect(SigC::slot(*this, &Game::actionTriggered));

    r->map("endgame", SigC::slot(*this, & Game::endGame));
    r->map("debug", SigC::slot(*this, & Game::toggleDebugMode));

    r->setAxis("kbd_throttle",-1.0f);
    r->map("throttle0", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.0f));
    r->map("throttle1", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.11f));
    r->map("throttle2", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.22f));
    r->map("throttle3", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.33f));
    r->map("throttle4", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.44f));
    r->map("throttle5", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.55f));
    r->map("throttle6", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.66f));
    r->map("throttle7", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.77f));
    r->map("throttle8", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.88f));
    r->map("throttle9", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 1.0f));

    r->map("autopilot", SigC::slot(*this, &Game::toggleControlMode));

    r->map("pause", SigC::slot(*this, & Game::togglePauseMode));
    r->map("view0", SigC::bind(
    	SigC::slot(*this, &Game::setView), 0));
    r->map("view1", SigC::bind(
    	SigC::slot(*this, &Game::setView), 1));
    r->map("view2", SigC::bind(
    	SigC::slot(*this, &Game::setView), 2));
    r->map("view3", SigC::bind(
    	SigC::slot(*this, &Game::setView), 3));
    r->map("view4", SigC::bind(
    	SigC::slot(*this, &Game::setView), 4));
    r->map("view5", SigC::bind(
    	SigC::slot(*this, &Game::setView), 5));
    r->map("next-view-subject", SigC::slot(*this, &Game::nextTarget));
    r->map("external-view", SigC::slot(*this, &Game::externalView));

    r->map("faster", SigC::slot(*this, &Game::accelerateSpeed));
    r->map("slower", SigC::slot(*this, &Game::decelerateSpeed));
    
    r->addAxisManipulator(
        AxisManipulator(new LinearAxisTransform(-0.5f, 0.5f), "js_throttle2")
        .input("js_throttle"));

    r->addAxisManipulator(
        AxisManipulator(new LinearAxisTransform(1.0f/5, 0.0f), "mouse_aileron")
        .input("mouse_rel_x"));
    r->addAxisManipulator(
        AxisManipulator(new LinearAxisTransform(1.0f/5, 0.0f), "mouse_elevator")
        .input("mouse_rel_y"));

    r->addAxisManipulator(
        AxisManipulator(new ClampAxisTransform(), "mouse_aileron")
        .input("mouse_aileron"));
    r->addAxisManipulator(
        AxisManipulator(new ClampAxisTransform(), "mouse_elevator")
        .input("mouse_elevator"));
        
    r->map("+left",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_aileron_left", -1.0f));
    r->map("-left",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_aileron_left", 0.0f));
    r->map("+right",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_aileron_right", 1.0f));
    r->map("-right",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_aileron_right", 0.0f));
    r->map("+forward",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_elevator_up", -1.0f));
    r->map("-forward",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_elevator_up", 0.0f));
    r->map("+backward",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_elevator_down", +1.0f));
    r->map("-backward",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_elevator_down", 0.0f));

    r->addAxisManipulator(
        AxisManipulator(new SumAxesTransform(), "aileron")
        .input("mouse_aileron")
        .input("js_aileron")
        .input("kbd_aileron_left")
        .input("kbd_aileron_right"));
    r->addAxisManipulator(
        AxisManipulator(new SumAxesTransform(), "elevator")
        .input("mouse_elevator")
        .input("js_elevator")
        .input("kbd_elevator_up")
        .input("kbd_elevator_down"));
    r->addAxisManipulator(
        AxisManipulator(new SumAxesTransform(), "rudder")
        .input("js_rudder"));
    r->addAxisManipulator(
        AxisManipulator(new SelectAxisByActivityTransform(0.05f, 1.0f), "throttle")
        .input("js_throttle2")
        .input("kbd_throttle"));
        
    r->map("+forward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_throttle", 1.0f));
    r->map("-forward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_throttle", 0.0f));
    r->map("+backward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_brake", 1.0f));
    r->map("-backward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_brake", 0.0f));
    r->map("+left", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_steer_left", -1.0f));
    r->map("-left", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_steer_left", 0.0f));
    r->map("+right", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_steer_right", 1.0f));
    r->map("-right", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_steer_right", 0.0f));
    
    r->map("+forward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_up", 1.0f));
    r->map("-forward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_up", 0.0f));
    r->map("+backward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_down", -1.0f));
    r->map("-backward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_down", 0.0f));
    r->map("+left", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_left", -1.0f));
    r->map("-left", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_left", 0.0f));
    r->map("+right", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_right", 1.0f));
    r->map("-right", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_right", 0.0f));

    r->addAxisManipulator(
        AxisManipulator(new LinearAxisTransform(1.0f/5, 0.0f), "mouse_turret_steer")
        .input("mouse_rel_x"));
    r->addAxisManipulator(
        AxisManipulator(new LinearAxisTransform(-1.0f/5, 0.0f), "mouse_cannon_steer")
        .input("mouse_rel_y"));
    r->addAxisManipulator(
        AxisManipulator(new ClampAxisTransform(), "mouse_turret_steer")
        .input("mouse_turret_steer"));
    r->addAxisManipulator(
        AxisManipulator(new ClampAxisTransform(), "mouse_cannon_steer")
        .input("mouse_cannon_steer"));
    
    r->addAxisManipulator(
    	AxisManipulator(new SumAxesTransform(), "kbd_car_steer")
    	.input("kbd_car_steer_left")
    	.input("kbd_car_steer_right"));
    	
   	r->addAxisManipulator(
   		AxisManipulator(new SumAxesTransform(), "car_steer")
   		.input("kbd_car_steer"));
   	r->addAxisManipulator(
   		AxisManipulator(new SumAxesTransform(), "car_brake")
   		.input("kbd_car_brake"));
   	r->addAxisManipulator(
   		AxisManipulator(new SumAxesTransform(), "car_throttle")
   		.input("kbd_car_throttle"));

   	r->addAxisManipulator(
   		AxisManipulator(new SumAxesTransform(), "tank_turret_steer")
   		.input("mouse_turret_steer"));
   	r->addAxisManipulator(
   		AxisManipulator(new SumAxesTransform(), "tank_cannon_steer")
   		.input("mouse_cannon_steer"));
   		
    // Strafing
    r->addAxisManipulator(
    	AxisManipulator(new SumAxesTransform(), "strafe_horizontal")
    	.input("kbd_strafe_left")
    	.input("kbd_strafe_right"));
    r->addAxisManipulator(
    	AxisManipulator(new SumAxesTransform(), "strafe_vertical")
    	.input("kbd_strafe_up")
    	.input("kbd_strafe_down"));
}

void Game::doEvents()
{
    SDL_Event event;
    
    event_remapper->beginEvents();
    while(SDL_PollEvent(&event)) { // Loop while there are events on the queue
        event_remapper->feedEvent(event);
    }
    event_remapper->endEvents();
}

#define MAX_STEP_SECONDS (1.0 / 15.0)

void Game::preFrame()
{
    JCamera jcamera;

    doEvents();
    clock->update();

    soundman->update(clock->getFrameDelta());

    // this will return false if pause is activated
    while (clock->catchup(MAX_STEP_SECONDS)) {
        collisionman->run(this, clock->getStepDelta());
        cleanupActors();
        setupActors();
    }

    if (current_view) {
        camera->alignWith(&*current_view);
        camera->getCamera(&jcamera);
        renderer->setCamera(&jcamera.cam);
        Vector pos, up, right, front;
        pos = current_view->getLocation();
        current_view->getOrientation(&up, &right, &front);
        soundman->setListenerPosition(pos);
        soundman->setListenerOrientation(up, front);
        soundman->setListenerVelocity(current_view->getMovementVector());
    }

    environment->update(camera);
    setupRenderer();

    //SDL_Delay(5); // For audio processing;
}

#include <modules/actors/fx/explosion.h>

void Game::doFrame()
{
    setupRenderer();


#if ENABLE_HORIZON
    horizon->draw();
#endif

#if ENABLE_SKYBOX
    skybox->draw();
#endif

#if ENABLE_TERRAIN
    //ls_message("Drawing terrain\n");
    terrain->draw();
    //ls_message("/Drawing terrain\n");
#endif

#if ENABLE_LOD_TERRAIN
    quadman->draw();
#endif

#if ENABLE_DEBUG_TRIANGLE
    //drawDebugTriangle();
#endif

    drawActors();
}

void Game::postFrame()
{
#if ENABLE_GUNSIGHT
    if (gunsight) gunsight->draw();
#endif

#if ENABLE_MAP
    map->draw();
#endif

    console->draw(renderer);
    
    if (!clock->isPaused()) {
        IoObject* self = getProtoObject<Ptr<IGame> >(
            io_scripting_manager->getMainState());

        IoState_pushRetainPool(IOSTATE);
        if (IoObject_rawGetSlot_(self, IOSYMBOL("postFrame"))) {
            IoMessage *msg =
                IoMessage_newWithName_label_(IOSTATE, IOSYMBOL("postFrame"),IOSYMBOL("Game::postFrame"));
            IoState_tryToPerform(IOSTATE, self, IOSTATE->lobby, msg);
        }
        IoState_popRetainPool(IOSTATE);
    }
    
    clearScreen();
}

void Game::setupRenderer() {
    renderer->setClipRange(environment->getClipMin(),
                           environment->getClipMax());
    Vector col = environment->getFogColor();
    col *= 256.0;
    jcolor3_t fog_col;
    fog_col.r = col[0];
    fog_col.g = col[1];
    fog_col.b = col[2];
    //ls_message("setting fog color:\n"); col.dump();
    renderer->setFogColor(&fog_col);
    //renderer->setFogType(JR_FOGTYPE_FARAWAY,0.0f);
    renderer->setFogType(JR_FOGTYPE_LINEAR,0.0f);
    //renderer->setFogType(JR_FOGTYPE_EXP_SQUARE, 0.25);
    renderer->enableFog();
    renderer->setBackgroundColor(&fog_col);

    //clearScreen();
}

void Game::drawDebugTriangle()
{
    jvertex_col v1={{ 0.0f,INITIAL_ALTITUDE+0.4,15.0f},{255.0f,255.0f,255.0f}};
    jvertex_col v2={{ 0.4f,INITIAL_ALTITUDE-0.3,15.0f},{128.0f,128.0f,128.0f}};
    jvertex_col v3={{-0.4f,INITIAL_ALTITUDE-0.3,15.0f},{0.0f,0.0f,0.0f}};

    renderer->flush();

    renderer->setCullMode(JR_CULLMODE_CULL_NEGATIVE);
    renderer->enableAlphaBlending();
    renderer->setVertexMode(JR_VERTEXMODE_GOURAUD);
    renderer->begin(JR_DRAWMODE_TRIANGLES);
    renderer->setAlpha(255.0);
    renderer->addVertex(&v1);
    renderer->setAlpha(0.0);
    renderer->addVertex(&v2);
    renderer->setAlpha(128.0);
    renderer->addVertex(&v3);
    renderer->end();
    renderer->flush();
    renderer->disableAlphaBlending();
}

#define PI 3.14159265358979323846
#define REL_SIZE 100.0
void Game::drawDebugTriangleAt(const Vector & p)
{
    //ls_message("Drawing debug triangle at ");
    //Vector(p).dump();

    jvertex_col v1={{ 0.0f * REL_SIZE,0.4f * REL_SIZE,0.0f},{255.0f,255.0f,255.0f}};
    jvertex_col v2={{ 0.4f * REL_SIZE,-0.3f * REL_SIZE,0.0f},{128.0f,128.0f,128.0f}};
    jvertex_col v3={{-0.4f * REL_SIZE,-0.3f * REL_SIZE,0.0f},{0.0f,0.0f,0.0f}};

    renderer->flush();
    v1.p.x += p[0]; v1.p.y += p[1]; v1.p.z += p[2];
    v2.p.x += p[0]; v2.p.y += p[1]; v2.p.z += p[2];
    v3.p.x += p[0]; v3.p.y += p[1]; v3.p.z += p[2];

    renderer->setCullMode(JR_CULLMODE_CULL_NEGATIVE);
    renderer->enableAlphaBlending();
    renderer->setVertexMode(JR_VERTEXMODE_GOURAUD);
    renderer->begin(JR_DRAWMODE_TRIANGLES);
    renderer->setAlpha(255.0);
    renderer->addVertex(&v1);
    renderer->setAlpha(0.0);
    renderer->addVertex(&v2);
    renderer->setAlpha(128.0);
    renderer->addVertex(&v3);
    renderer->end();
    renderer->flush();
    renderer->disableAlphaBlending();
}

void Game::clearScreen() {
    SDL_GL_SwapBuffers();
    renderer->clear();
}

void Game::togglePauseMode() {
    if (clock->isPaused()) clock->resume(); else clock->pause();
}

void Game::toggleDebugMode() {
    debug_mode = !debug_mode;
}

void Game::endGame() {
    game_done=true;
}

void Game::accelerateSpeed() {
    clock->setTimeFactor(clock->getTimeFactor() * 1.5);
}

void Game::decelerateSpeed() {
    clock->setTimeFactor(clock->getTimeFactor() / 1.5);
}

void Game::setView(int n) {
    if (view_is_external)
        return;
	if (!current_view || !current_view->getViewSubject()) return;
	if (n >= current_view->getViewSubject()->getNumViews()) return;
	setCurrentView(current_view->getViewSubject()->getView(n));
}

void Game::externalView() {
	if (view_is_external) {
        setCurrentView(previous_view);
        view_is_external = false;
        if (current_view->getViewSubject()->hasControlMode(IActor::MANUAL)) {
            current_view->getViewSubject()->setControlMode(IActor::MANUAL);
        }
    } else {
        previous_view = current_view;
        
        Ptr<Observer> observer = new Observer(this);
        if (current_view) {
            observer->setLocation(current_view->getLocation());
            Vector up, right, front;
            current_view->getOrientation(&up, &right, &front);
            observer->setOrientation(up, right, front);
            if (current_view->getViewSubject()->hasControlMode(IActor::AUTOMATIC)) {
                current_view->getViewSubject()->setControlMode(IActor::AUTOMATIC);
            } else {
                current_view->getViewSubject()->setControlMode(IActor::UNCONTROLLED);
            }
        }
        addWeakActor(ptr(observer));
		setCurrentView(observer->getView(0));
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
    
    List & list = actors;
    Iter current = list.begin();
    if(current_view && current_view->getViewSubject()) {
    	current = find(list.begin(),list.end(), 
    		current_view->getViewSubject());
    }
    if (current == list.end()) current = list.begin();
    if (current==list.end()) return;
    
    if (debugMode()) {
    	current++;
    } else {
    	Iter next = current;
    	while (++next != current) {
    		if (next == list.end()) next = list.begin();
    		if ((*next)->hasControlMode(IActor::MANUAL))
    			break;
    	}
    	current = next;
    }
    if (current == list.end()) current = list.begin();
    
    ls_message("Game: Switching to %s at: ", typeid(*(*current)).name());
    (*current)->getLocation().dump();
    setCurrentView((*current)->getView(0));
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

    IoMessage *message = IoMessage_newWithName_label_(IOSTATE, IOSYMBOL("sendMessage"),
        IOSYMBOL("Game::actionTriggered"));
    
    IoMessage_addCachedArg_(message, IOSYMBOL(action));
    IoMessage_addCachedArg_(message, IONIL(self));

    
    IoObject * result = IoState_tryToPerform(IOSTATE, self, IOSTATE->lobby, message);
    IoState_stackRetain_(IOSTATE, result);
}

int main(int argc, char *argv[])
{
    try {
        Game *game = new Game(argc, (const char**)argv);
        game->run();
    } catch (std::exception & e) {
        ls_error("Caught exception: %s\n", e.what());
#ifndef NDEBUG
        // Pass on the exception to the debugger
        throw;
#endif
    }
    return 0;
}

