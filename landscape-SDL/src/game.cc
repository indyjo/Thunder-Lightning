#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <algorithm>
#include <sigc++/bind.h>
#include <modules/math/Vector.h>
#include <modules/config/config.h>
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

#include <sound.h>
#include <Faction.h>
#include "game.h"

#include <stdexcept>

#define RAND ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)

using namespace std;

Game * Game::the_game = 0;


Game::Game(int argc, const char **argv)
: argc(argc), argv(argv), game_speed(1.0)
{
    the_game = this;

    ls_message("Initializing configuration system:\n");
    config= new Config;
    config->feedArguments(argc, argv);
    ls_message("Continuing initialization from Io language:\n");
    io_scripting_manager = new IoScriptingManager(this);
    addBasicMappings(this, io_scripting_manager->getMainState());
    {
		char buf[256];
		strncpy(buf,config->query("Io_init_script"),256);
		IoState_doFile_(io_scripting_manager->getMainState(), buf);
    }
    ls_message("done.\n");

    ls_message("Initializing SDL: ");
    if (-1 == SDL_Init( SDL_INIT_VIDEO |
                        SDL_INIT_JOYSTICK |
                        (config->queryBool("Game_enable_SDL_parachute", true)?
                        	0:SDL_INIT_NOPARACHUTE) )  )
    {
        const char * err = SDL_GetError();
        ls_error("error: %s", err);
    }
    SDL_EnableUNICODE(true);
    ls_message("Found %d joysticks.\n", SDL_NumJoysticks());
    SDL_JoystickEventState(SDL_ENABLE);
    for(int i=0; i<SDL_NumJoysticks(); i++) {
        SDL_JoystickOpen(i);
    }
    ls_message("done.\n");

    ls_message("Initializing video: ");
    {
        int xres = atoi( config->query("Game_xres", "1024") );
        int yres = atoi( config->query("Game_yres", "768") );
        bool fullscreen = atoi( config->query("Game_fullscreen", "0") );

        ls_message("Requested mode: %dx%d %s\n",
                xres, yres,
                fullscreen?"(fullscreen)":"(in a window)");

        SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
        SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
        SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
        if(fullscreen) {
            surface = SDL_SetVideoMode(xres, yres, 32,
                    SDL_OPENGL | SDL_FULLSCREEN);
        } else {
            surface = SDL_SetVideoMode(xres, yres, 32, SDL_OPENGL);
        }
        if (!surface) ls_error("error! ");
        int r=0,g=0,b=0,d=0,db=0;
        SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &r );
        SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &g );
        SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &b );
        SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &d );
        SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &db );
        ls_message(" got r/g/b/d %d/%d/%d/%d %s double buffering. ",
                r,g,b,d,db?"width":"without");
        renderer = new JOpenGLRenderer(xres, yres, atof(config->query("Camera_aspect","1.33333")));
    	camera = new Camera(this);
    	JCamera jcam;
    	camera->getCamera(&jcam);
    	renderer->setCamera(&jcam.cam);
    	renderer->setClipRange(0.1, 10);
    }
    ls_message("done.\n");

    pause = false;

    mouse_relx=mouse_rely=0;
    mouse_buttons=0;
    
    if (config->query("Game_grab_mouse","true") == std::string("true")) {
	    SDL_WM_GrabInput(SDL_GRAB_ON);
	    SDL_ShowCursor(SDL_DISABLE);
    }

    key_tab_old=false;

    ls_message("Initializing managers... ");
    texman = new TextureManager(*renderer);
    event_remapper = new EventRemapper();
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
        initControls();
        stat.endJob();
    }
    //renderer->setClipRange(CLIP_MIN_RANGE, CLIP_RANGE);

    /*
    Ptr<Drone> drone;
    Ptr<SmokeTrail> smoke;

    for(int i=0; i<1; i++) {
        drone = new Drone(this);
        Vector p = Vector(
            1000.0f*RAND,
            0.0f,
            1000.0f*RAND + 1000);
        p[1] = std::max(getTerrain()->getHeightAt(p[0],p[2])+300.0f, INITIAL_ALTITUDE);
        Vector v = Vector(0,0,0);
        drone->setLocation(p);
        drone->setMovementVector(v);
        drone->setControlMode(IActor::AUTOMATIC);
        float r = RAND;
        if (3*r<1) drone->setFaction(Faction::basic_factions.faction_a);
        else if (3*r<2) drone->setFaction(Faction::basic_factions.faction_b);
        else drone->setFaction(Faction::basic_factions.faction_c);

        addActor( drone );
        //smoke = new SmokeTrail(this);
        //smoke->follow(drone);
        //addActor( smoke );
    }
    
    setCurrentView(drone->getView(0));
    setCurrentlyControlledActor(drone);*/

    /*
    for(int i=0; i<25; ++i) {
        Ptr<Tank> tank = new Tank(this);
        
        float r = RAND;
        if (3*r<1) tank->setFaction(Faction::basic_factions.faction_a);
        else if (3*r<2) tank->setFaction(Faction::basic_factions.faction_b);
        else tank->setFaction(Faction::basic_factions.faction_c);
        
        tank->setLocation(Vector(RAND*15000,0,RAND*15000));
        addActor(tank);
    }
    */
    /*
    Ptr<RigidActor> a = new RigidActor(
    	this,
    	collisionman->queryGeometry(
        	(std::string()+config->query("model_dir")+
    			"/asteroid/asteroid.bounds").c_str()
        ));
    a->setModel(modelman->query(
    	(std::string()+config->query("model_dir")+
    		"/asteroid/asteroid.obj").c_str()));
    a->setLocation(Vector(0, 500, 1000));
    a->getEngine()->construct(1e5, 1e7, 1e7, 5e6);
    //a->getEngine()->applyAngularImpulse(Vector(10000,100,5000));
    Ptr<TargetInfo> ti = new TargetInfo("Asteroid", 50, TargetInfo::AIRCRAFT);
    a->setTargetInfo(ti);
    addActor(a);
    
	a = new RigidActor(
    	this,
    	collisionman->queryGeometry(
        	(std::string()+config->query("model_dir")+
    			"/asteroid/asteroid.bounds").c_str()
        ));
    a->setModel(modelman->query(
    	(std::string()+config->query("model_dir")+
    		"/asteroid/asteroid.obj").c_str()));
    a->setLocation(Vector(00, 500, 2040));
    a->getEngine()->construct(1e5, 1e7, 1e7, 5e6);
    a->getEngine()->setAngularMomentum(Vector(-1e7,1e5, 1e6));
    a->getEngine()->setLinearMomentum(Vector(0,0,-1e7));
    a->setTargetInfo(new TargetInfo("Asteroid2", 50, TargetInfo::AIRCRAFT));
    addActor(a);
    */
    
    console = new UI::Console(this, getScreenSurface());
    addMappings(this, io_scripting_manager->getMainState());
    {
		char buf[256];
		strncpy(buf,config->query("Io_init_script_2"),256);
		IoState_doFile_(io_scripting_manager->getMainState(), buf);
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

    doEvents();
    while (!game_done) {
        preFrame();
        doFrame();
        postFrame();
    }
    
    //Object::debug();
    
    removeAllActors();
    current_view = 0;
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
    float xres = atof(config->query("Game_xres","1024"));
    float yres = atof(config->query("Game_yres","768"));
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
    ls_message("setGunsight\n");
    this->gunsight=gunsight;
    ls_message("end setGunsight\n");
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

void Game::getMouseState(float *mx, float *my, int *buttons)
{
    *mx=mouse_relx;
    *my=mouse_rely;
    *buttons=mouse_buttons;
}

double Game::getTimeDelta()
{
    //return (double) (ticks_now - ticks_old) * game_speed;
    //return timeaccum;
    return clock->getStepDelta() * 1000.0;
}

double Game::getTime()
{
    return (double) 0.0;
}

Ptr<IView> Game::getCurrentView()
{
	return current_view;
}

void Game::setCurrentView(Ptr<IView> view)
{
	current_view = view;
	if (view) setGunsight(view->getGunsight());
	else setGunsight(0);
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
    r->map("endgame", SigC::slot(*this, & Game::endGame));

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
        AxisManipulator(new SelectAxisByActivityTransform(0.01f), "throttle")
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
   	
}

void Game::doEvents()
{
    SDL_Event event;
    int x,y;
    int dummy;

    mouse_relx = mouse_rely = 0;
    mouse_buttons = 0;

    event_remapper->beginEvents();
    while(SDL_PollEvent(&event)) { // Loop while there are events on the queue
        switch(event.type) {
        case SDL_KEYDOWN:
            //ls_message("Key pressed: %s\n",
            //        SDL_GetKeyName(event.key.keysym.sym));
            //keyboard_sig.emit(event.key.keysym.sym, true);
            break;
        case SDL_KEYUP:
            //ls_message("Key released: %s\n",
            //        SDL_GetKeyName(event.key.keysym.sym));
            //keyboard_sig.emit(event.key.keysym.sym, false);
            break;
        case SDL_MOUSEMOTION:
            mouse_relx += event.motion.xrel;
            mouse_rely += event.motion.yrel;
            //ls_message("Mouse motion: %d %d\n", mouse_relx, mouse_rely);
            break;
        case SDL_MOUSEBUTTONDOWN:
            mouse_buttons |= event.button.button;
            break;
        }
        event_remapper->feedEvent(event);
    }
    event_remapper->endEvents();
}

// void Game::doTime()
// {
//     timeval tval;
//     double timedelta;
//
//     gettimeofday(&tval,0);
//     timeold=timenow;
//     //ls_message("Gettimeofday: %d.%d\n", tval.tv_sec, tval.tv_usec);
//     timenow=(tval.tv_sec)*1000.0+(tval.tv_usec)/1000.0;
//     //ls_message("Current timenow: %f\n", timenow);
//     if (pause) {
//         timeold = timenow;
//         timedelta = 0;
//         timeaccum=0;
//     } else {
//         timedelta=timenow-timeold;
//         timeaccum=(timeaccum*(TIME_ACCUM-1.0)+timedelta)/TIME_ACCUM;
//     }
//     //ls_message("Time since last frame (accum): %f (real): %f\n",timeaccum,timedelta);
// }

#define MAX_STEP_SECONDS (1.0 / 15.0)

void Game::preFrame()
{
    JCamera jcamera;

    doEvents();
    clock->update();

    soundman->update();

    // this will return false if pause is activated
    while (clock->catchup(MAX_STEP_SECONDS)) {
        if (clock->getStepDelta() > MAX_STEP_SECONDS) {
            ls_warning("!!!!!!\a\a\a%f\n", clock->getStepDelta());
        }

        collisionman->run(this, clock->getStepDelta());
        /*
        if(clock->isPaused()) {
            while(clock->catchup(MAX_STEP_SECONDS)) ;
            continue;
        }
        */

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
    
    IoObject* self = getProtoObject<IGame>(
    	io_scripting_manager->getMainState());
    if (IoObject_rawGetSlot_(self, IOSTRING("postFrame"))) {
		IoState_pushRetainPool(IOSTATE);
		IoMessage *msg =
			IoMessage_newWithName_(IOSTATE, IOSTRING("postFrame"));
		IoState_stackRetain_(IOSTATE, msg);
		IoMessage_locals_performOn_(msg,IOSTATE->lobby,self);
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
	if (!current_view || !current_view->getViewSubject()) return;
	if (n >= current_view->getViewSubject()->getNumViews()) return;
	setCurrentView(current_view->getViewSubject()->getView(n));
}

void Game::externalView() {
	if (current_view) {
		setCurrentView(0);
	} else {
		if (!current_actor)
			return;
		setCurrentView(current_actor->getView(0));
	}
}

void Game::nextTarget() {
    typedef const std::vector<Ptr<IActor> > List;
    typedef List::const_iterator Iter;
    
    //if (!current_view || !current_view->getViewSubject()) return;
    
    List & list = actors;
    Iter current = list.begin();
    if(current_view && current_view->getViewSubject()) {
    	current = find(list.begin(),list.end(), 
    		current_view->getViewSubject());
    }
    if (current == list.end()) current = list.begin();
    if (current==list.end()) return;
    current++;
    if (current == list.end()) current = list.begin();
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

int main(int argc, char *argv[])
{
    try {
        Game *game = new Game(argc, (const char**)argv);
        game->run();
    } catch (std::exception & e) {
        ls_error("Uncaught exception: %s\n", e.what());
    }
    return 0;
}
