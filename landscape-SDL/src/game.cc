#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <sigc++/bind.h>
#include <modules/math/Vector.h>
#include <modules/config/config.h>
#include <modules/camera/camera.h>
#include <modules/clock/clock.h>
#include <modules/player/player.h>
#include <modules/LoDTerrain/LoDTerrain.h>
#include <modules/skybox/skybox.h>
#include <modules/map/map.h>
#include <modules/gunsight/gunsight.h>
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

#include <sound.h>
#include <Faction.h>
#include "game.h"

#include <stdexcept>

#define RAND ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)

using namespace std;

Game * Game::the_game = 0;


Game::Game(int argc, const char **argv)
: argc(argc), argv(argv), game_speed(1.0), view(0)
{
    the_game = this;

    ls_message("Initializing configuration system: ");
    config= new Config;
    config->feedArguments(argc, argv);
    ls_message("done.\n");

    ls_message("Initializing SDL: ");
    if (-1 == SDL_Init( SDL_INIT_VIDEO |
                        SDL_INIT_JOYSTICK |
                        SDL_INIT_NOPARACHUTE )  )
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
        int r,g,b,d,db;
        SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &r );
        SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &g );
        SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &b );
        SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &d );
        SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &db );
        ls_message(" got r/g/b/d %d/%d/%d/%d %s double buffering. ",
                r,g,b,d,db?"width":"without");
        renderer = new JOpenGLRenderer(xres, yres);
    }
    ls_message("done.\n");

    pause = false;
    follow_mode = false;

    mouse_relx=mouse_rely=0;
    mouse_buttons=0;
    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(SDL_DISABLE);

    key_tab_old=false;

    ls_message("Initializing managers... ");
    texman = new TextureManager(*renderer);
    event_remapper = new EventRemapper();
    modelman = new ModelMan(texman);
    fontman = new FontMan(this);
    soundman = new SoundMan(config->query("SoundMan_sound_dir"));
    collisionman = new Collide::CollisionManager();
    clock = new Clock;
    ls_message("done.\n");

    {
        string background = config->query("Game_loading_screen");
        LoadingScreen lscr(this, background);
        Status stat;
        stat.getSignal().connect(SigC::slot(lscr, &LoadingScreen::update));

        stat.beginJob("Initializing", 2);
        initModules(stat);
        stat.stepFinished();
        initControls();
        stat.endJob();
    }
    //renderer->setClipRange(CLIP_MIN_RANGE, CLIP_RANGE);

    cam_pos = player;

    Ptr<Drone> drone;
    Ptr<SmokeTrail> smoke;

    for(int i=0; i<3; i++) {
        ls_message("create drone.\n");
        drone = new Drone(this);
        Vector p = Vector(
            1000.0f*RAND,
            0.0f,
            1000.0f*RAND + 1000);
        p[1] = std::max(getTerrain()->getHeightAt(p[0],p[2])+300.0f, INITIAL_ALTITUDE);
        Vector v = Vector(0,0,0);
        drone->setLocation(p);
        drone->setMovementVector(v);
        float r = RAND;
        if (3*r<1) drone->setFaction(Faction::basic_factions.faction_a);
        else if (3*r<2) drone->setFaction(Faction::basic_factions.faction_b);
        else drone->setFaction(Faction::basic_factions.faction_c);

        ls_message("add drone.\n");
        addActor( drone );
        ls_message("done.\n");
        //smoke = new SmokeTrail(this);
        //smoke->follow(drone);
        //addActor( smoke );
    }

    for(int i=0; i<5; ++i) {
        Ptr<Tank> tank = new Tank(this);
        tank->setLocation(Vector(RAND*5000,0,RAND*5000));
        addActor(tank);
    }

    float aspect = 1.3;
    float focus = camera->getFocus();
    UI::Surface surface =
        UI::Surface::FromCamera(aspect, focus, 1024, 768);
    console = new UI::Console(this, surface);

    while (clock->catchup(1.0f)) ;
}

Game::~Game()
{
    the_game = 0;

    delete event_remapper;
    ls_message("Exiting SDL.\n");
    SDL_Quit();
    ls_message("Exiting game.\n");
}

void Game::run()
{
    game_done=false;

    doEvents();
    while (!game_done) {
        preFrame();
        doFrame();
        postFrame();
    }
}


Ptr<TextureManager> Game::getTexMan() {
    return texman;
}

JRenderer *Game::getRenderer()
{
    return renderer;
}

EventRemapper *Game::getEventRemapper()
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

Ptr<IActor> Game::getCamPos()
{
    return cam_pos;
}

void Game::setCamPos(Ptr<IActor> c)
{
    cam_pos=c;
    if (!c) cam_pos = gunsight->getCurrentTarget();
}

Ptr<ICamera> Game::getCamera()
{
    return camera;
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
#if ENABLE_TERRAIN
    return terrain;
#endif
}

Ptr<IPlayer> Game::getPlayer()
{
    return player;
}

Ptr<IGunsight> Game::getGunsight()
{
    return gunsight;
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

void Game::initModules(Status & stat)
{
    int steps = 2;
#define STEPS 2
#if ENABLE_TERRAIN
    steps++;
#endif
#if ENABLE_LOD_TERRAIN
    steps++;
#endif
#if ENABLE_SKY
    steps++;
#endif
#if ENABLE_SKYBOX
    steps++;
#endif
#if ENABLE_HORIZON
    steps++;
#endif
#if ENABLE_MAP
    steps++;
#endif
#if ENABLE_GUNSIGHT
    steps++;
#endif

    stat.beginJob("Initialize modules", steps);

    camera = new Camera(this);
    environment = new Environment();
    stat.stepFinished();
#if ENABLE_TERRAIN
    terrain = new Terrain(this);
    stat.stepFinished();
    //Terrain *terrain = new Terrain(this);
    ls_message("Game@%p drawing Terrain@%p\n", this, &*terrain);
    terrain->draw();
#endif
#if ENABLE_LOD_TERRAIN
    stat.beginJob("Initialize LOD terrain",1);
    quadman = new LoDQuadManager(this, stat);
    stat.endJob();
#endif
    ls_message("init player.\n");
    player = new Player(this);
    ls_message("adding player.\n");
    addActor(player);
    ls_message("init player done.\n");
    stat.stepFinished();
#if ENABLE_SKY
    sky = new Sky(this);
    stat.stepFinished();
#endif
#if ENABLE_SKYBOX
    skybox = new SkyBox(this);
    stat.stepFinished();
#endif
#if ENABLE_HORIZON
    horizon = new Horizon(this);
    stat.stepFinished();
#endif
#if ENABLE_MAP
    map = new Map(this);
    stat.stepFinished();
#endif
#if ENABLE_GUNSIGHT
    gunsight = new Gunsight(this);
    stat.stepFinished();
#endif

    stat.endJob();
}

namespace {
    template<class T> void setvar(T * x, const T v) { *x = v; }
}

void Game::initControls()
{
    EventRemapper *r = getEventRemapper();
    r->mapKey(SDLK_ESCAPE, true, "endgame");
    r->map("endgame", SigC::slot(*this, & Game::endGame));

    r->mapKey(SDLK_TAB, true,  "map_magnify");
    r->mapKey(SDLK_TAB, false, "map_demagnify");

    r->mapKey(SDLK_w, true,  "+strafe_forward");
    r->mapKey(SDLK_w, false, "-strafe_forward");
    r->mapKey(SDLK_s, true,  "+strafe_backward");
    r->mapKey(SDLK_s, false, "-strafe_backward");
    r->mapKey(SDLK_a, true,  "+strafe_left");
    r->mapKey(SDLK_a, false, "-strafe_left");
    r->mapKey(SDLK_d, true,  "+strafe_right");
    r->mapKey(SDLK_d, false, "-strafe_right");

    r->mapKey(SDLK_UP, true, "+forward");
    r->mapKey(SDLK_UP, false, "-forward");
    r->mapKey(SDLK_DOWN, true, "+backward");
    r->mapKey(SDLK_DOWN, false, "-backward");
    r->mapKey(SDLK_LEFT, true, "+left");
    r->mapKey(SDLK_LEFT, false, "-left");
    r->mapKey(SDLK_RIGHT, true, "+right");
    r->mapKey(SDLK_RIGHT, false, "-right");

    r->mapKey(SDLK_1, true, "throttle0");
    r->map("throttle0", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.0f));
    r->mapKey(SDLK_2, true, "throttle1");
    r->map("throttle1", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.11f));
    r->mapKey(SDLK_3, true, "throttle2");
    r->map("throttle2", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.22f));
    r->mapKey(SDLK_4, true, "throttle3");
    r->map("throttle3", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.33f));
    r->mapKey(SDLK_5, true, "throttle4");
    r->map("throttle4", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.44f));
    r->mapKey(SDLK_6, true, "throttle5");
    r->map("throttle5", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.55f));
    r->mapKey(SDLK_7, true, "throttle6");
    r->map("throttle6", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.66f));
    r->mapKey(SDLK_8, true, "throttle7");
    r->map("throttle7", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.77f));
    r->mapKey(SDLK_9, true, "throttle8");
    r->map("throttle8", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.88f));
    r->mapKey(SDLK_0, true, "throttle9");
    r->map("throttle9", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 1.0f));

    r->mapKey(SDLK_HOME, true, "autopilot");

    r->mapKey(SDLK_p, true, "pause");
    r->map("pause", SigC::slot(*this, & Game::togglePauseMode));
    r->mapKey(SDLK_CARET, true, "toggle-console");

    r->mapKey(SDLK_F1, true, "view0");
    r->mapKey(SDLK_F2, true, "view1");
    r->mapKey(SDLK_F3, true, "view2");
    r->mapKey(SDLK_F4, true, "view3");
    r->mapKey(SDLK_F5, true, "view4");
    r->mapKey(SDLK_F6, true, "view5");
    r->map("view0", SigC::bind( SigC::slot(&setvar<int>), &view, 0));
    r->map("view1", SigC::bind( SigC::slot(&setvar<int>), &view, 1));
    r->map("view2", SigC::bind( SigC::slot(&setvar<int>), &view, 2));
    r->map("view3", SigC::bind( SigC::slot(&setvar<int>), &view, 3));
    r->map("view4", SigC::bind( SigC::slot(&setvar<int>), &view, 4));
    r->map("view5", SigC::bind( SigC::slot(&setvar<int>), &view, 5));


    r->mapKey(SDLK_v, true, "external");
    r->map("external", SigC::bind(
            SigC::slot(*this, & Game::setCamPos),
            Ptr<IActor>()));
    r->mapKey(SDLK_f, true, "follow");
    r->map("follow", SigC::slot(*this, & Game::toggleFollowMode));

    r->mapKey(SDLK_BACKSPACE, true, "cycle-primary");
    r->mapKey(SDLK_RETURN,    true, "cycle-secondary");
    r->mapKey(SDLK_LCTRL, true,  "+primary");
    r->mapKey(SDLK_LCTRL, false, "-primary");
    r->mapKey(SDLK_SPACE, true,  "+secondary");
    r->mapKey(SDLK_SPACE, false, "-secondary");
    r->mapKey(SDLK_RCTRL, true,  "+tertiary");
    r->mapKey(SDLK_RCTRL, false, "-tertiary");
    
    r->mapMouseButton(1, true,  "+primary");
    r->mapMouseButton(1, false, "-primary");
    r->mapMouseButton(3, true,  "+secondary");
    r->mapMouseButton(3, false, "-secondary");
    r->mapMouseButton(2, true,  "+tertiary");
    r->mapMouseButton(2, false, "-tertiary");
    r->mapMouseButton(4, true,  "cycle-secondary");
    r->mapMouseButton(5, true,  "cycle-secondary");
    r->mapJoystickButton(0, 0, true,  "+primary");
    r->mapJoystickButton(0, 0, false, "-primary");
    r->mapJoystickButton(0, 1, true,  "+secondary");
    r->mapJoystickButton(0, 1, false, "-secondary");
    r->mapJoystickButton(0, 2, true,  "gunsight_target");
    r->mapJoystickButton(0, 3, true,  "cycle-secondary");


    r->mapKey(SDLK_F12, true, "debug");

    r->mapKey(SDLK_t, true, "next_target");
    r->mapKey(SDLK_g, true, "gunsight_target");
    r->mapKey(SDLK_h, true, "toggle_gunsight_info");

    r->mapKey(SDLK_F9, true, "faster");
    r->mapKey(SDLK_F10, true, "slower");
    r->map("faster", SigC::slot(*this, &Game::accelerateSpeed));
    r->map("slower", SigC::slot(*this, &Game::decelerateSpeed));

    r->mapJoystickAxis(0,0,"js_aileron");
    r->mapJoystickAxis(0,1,"js_elevator");
    r->mapJoystickAxis(0,2,"js_rudder");
    r->mapJoystickAxis(0,3,"js_throttle");
    
    r->addAxisManipulator(
        AxisManipulator(new LinearAxisTransform(-0.5f, 0.5f), "js_throttle2")
        .input("js_throttle"));

    r->mapRelativeMouseAxes("mouse_rel_x", "mouse_rel_y");
    r->mapAbsoluteMouseAxes("mouse_abs_x", "mouse_abs_y");

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

    /*
    r->mapKey(SDLK_UP, true, "cannon-up");
    r->mapKey(SDLK_DOWN, true, "cannon-down");
    r->mapKey(SDLK_UP, false, "cannon-idle");
    r->mapKey(SDLK_DOWN, false, "cannon-idle");
    r->mapKey(SDLK_LEFT, true, "turret-left");
    r->mapKey(SDLK_LEFT, false, "turret-idle");
    r->mapKey(SDLK_RIGHT, true, "turret-right");
    r->mapKey(SDLK_RIGHT, false, "turret-idle");
    r->mapKey(SDLK_i, true, "+throttle");
    r->mapKey(SDLK_i, false, "-throttle");
    r->mapKey(SDLK_k, true, "+brake");
    r->mapKey(SDLK_k, false, "-brake");
    r->mapKey(SDLK_j, true, "steer-left");
    r->mapKey(SDLK_j, false, "steer-idle");
    r->mapKey(SDLK_l, true, "steer-right");
    r->mapKey(SDLK_l, false, "steer-idle");
    r->mapKey(SDLK_RETURN, true, "+cannon-fire");
    r->mapKey(SDLK_RETURN, false, "-cannon-fire");
    */
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

        typedef ActorList::iterator Iter;
        for (Iter i=actors.begin(); i!=actors.end(); i++) {
            if (IActor::DEAD == (*i)->getState()) {
                actors.erase(i++);
            }
        }
        for (Iter i=actors.begin(); i!=actors.end(); i++) {
            (*i)->action();
        }
    }

    if (cam_pos && cam_pos->getState() == IActor::DEAD) cam_pos = 0;
    if (cam_pos) {
        Ptr<IPositionProvider> view_pos = cam_pos->getView(
                std::min(view, cam_pos->getNumViews()-1));
        camera->alignWith(&*view_pos);
        camera->getCamera(&jcamera);
        renderer->setCamera(&jcamera.cam);
        Vector pos, up, right, front;
        pos = view_pos->getLocation();
        view_pos->getOrientation(&up, &right, &front);
        soundman->setListenerPosition(pos);
        soundman->setListenerOrientation(up, front);
        soundman->setListenerVelocity(cam_pos->getMovementVector());
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

    ActorList::iterator i;

    for (i=actors.begin(); i!=actors.end(); i++) {
        (*i)->draw();
    }
}

void Game::postFrame()
{
    jcolor3_t sky_col;

#if ENABLE_SKY
    sky->draw();
#endif

#if ENABLE_GUNSIGHT
    gunsight->draw();
#endif

#if ENABLE_MAP
    map->draw();
#endif

#if ENABLE_SKY
    sky->getSkyColor(&sky_col);
    renderer->setBackgroundColor(&sky_col);
#endif

    console->draw(renderer);
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

void Game::clearScreen()
{
    SDL_GL_SwapBuffers();
    renderer->clear();
}

void Game::togglePauseMode() {
    if (clock->isPaused()) clock->resume(); else clock->pause();
}

void Game::toggleFollowMode() {
    follow_mode = !follow_mode;
    if (!follow_mode) cam_pos = player;
}

void Game::endGame()
{
    game_done=true;
}

void Game::accelerateSpeed() {
    clock->setTimeFactor(clock->getTimeFactor() * 1.5);
}

void Game::decelerateSpeed() {
    clock->setTimeFactor(clock->getTimeFactor() / 1.5);
}

int main(int argc, const char **argv)
{
    try {
        ls_message("Number of arguments: %d\n", argc);
        {
            Game g(argc, argv);
            g.run();
        }
        ls_message("Finished cleanly. GoodBye!\n");
    } catch (std::exception & e) {
        ls_error("Exception: %s\n", e.what());

    }
    return 0;
}
