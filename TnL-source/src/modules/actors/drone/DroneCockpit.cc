#include <cmath>
#include <sigc++/sigc++.h>
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <interfaces/IFont.h>
#include <interfaces/IFontMan.h>
#include <modules/camera/FollowingCamera.h>
#include <modules/camera/SimpleCamera.h>
#include <modules/environment/environment.h>
#include <modules/math/SpecialMatrices.h>
#include <modules/math/Transform.h>
#include <modules/model/model.h>
#include <modules/texman/Texture.h>
#include <modules/texman/TextureManager.h>
#include <modules/ui/Label.h>
#include <modules/ui/PanelRenderPass.h>
#include <modules/weaponsys/Armament.h>
#include <modules/weaponsys/Targeter.h>
#include <modules/weaponsys/Weapon.h>
#include <SceneRenderPass.h>
#include <TargetInfo.h>

#include "drone.h"

#include "DroneCockpit.h"

class MfdModule : public Object {
public:
    virtual ~MfdModule() { }
    
    virtual Ptr<RenderPass> createRenderPass(JRenderer *)=0;
};

class DummyModule : public MfdModule {
    WeakPtr<IGame> game;
public:
    DummyModule(WeakPtr<IGame> game) : game(game) { }

    virtual Ptr<RenderPass> createRenderPass(JRenderer * renderer) {
        Ptr<IGame> thegame = game.lock();
        if (!thegame) return new RenderPass(renderer);
        
        Ptr<RenderPass> background = new RenderPass(renderer);
        background->enableClearColor(true);
        background->setBackgroundColor(Vector(0,0.5,0));
        
        Ptr<IFontMan> fontman = thegame->getFontMan();
        fontman->selectFont(thegame->getConfig()->query("MFD_font_title", "default"));
        
        Ptr<UI::Panel> panel = new UI::Panel(renderer);
        Ptr<UI::StaticLabel> label = new UI::StaticLabel("label", "Dummy", fontman->getFont());
        label->setColor(Vector(0,1,0));
        label->setAlpha(1);
        panel->add(label, "root",
            UI::Panel::HCENTER|UI::Panel::VCENTER,
            UI::Panel::HCENTER|UI::Panel::VCENTER,
            Vector2(0,-0.2));
        
        Ptr<UI::PanelRenderPass> result = new UI::PanelRenderPass(renderer);
        result->setPanel(panel);
        result->stackedOn(background);
        return result;
    }
};

class MagViewModule : public MfdModule {
    WeakPtr<IGame> game;
    Ptr<IMovementProvider> pos;
public:
    MagViewModule(WeakPtr<IGame> game, Ptr<IMovementProvider> pos) : game(game), pos(pos) { }
    virtual Ptr<RenderPass> createRenderPass(JRenderer * renderer) {
        Ptr<IGame> thegame = game.lock();
        if (!thegame) return new RenderPass(renderer);
        
        Ptr<FollowingCamera> cam = new FollowingCamera();
        cam->setTarget(pos);
        cam->setFocus(25.0);
        cam->setAspect(1.0);
        cam->setNearDistance(5.0);
        cam->setFarDistance(thegame->getEnvironment()->getClipMax());
        
        RenderContext ctx(cam);
        ctx.draw_gunsight = false;
        ctx.draw_console = false;
        
        Ptr<SceneRenderPass> scene = new SceneRenderPass(thegame, ctx);
        
        Ptr<IFontMan> fontman = thegame->getFontMan();
        fontman->selectFont(thegame->getConfig()->query("MFD_font_title", "default"));
        
        Ptr<UI::Panel> panel = new UI::Panel(renderer);
        Ptr<UI::StaticLabel> label = new UI::StaticLabel("label", "Telescope", fontman->getFont());
        label->setColor(Vector(0,1,0));
        label->setAlpha(1);
        panel->add(label, "root",
            UI::Panel::HCENTER|UI::Panel::VCENTER,
            UI::Panel::HCENTER|UI::Panel::VCENTER,
            Vector2(0,-0.2));
        
        Ptr<UI::PanelRenderPass> result = new UI::PanelRenderPass(renderer);
        result->setPanel(panel);
        result->stackedOn(scene);
        return result;
    }
};

class MissileViewModule : public MfdModule, public SigC::Object {
    typedef std::vector<Ptr<Weapon> > Weapons;
    Weapons weapons;
    WeakPtr<IGame> game;
    Ptr<IWeapon> last_fired_weapon;
    
    class MissilePass : public RenderPass {
        WeakPtr<IGame> game;
        Ptr<IActor> round;
    public:
        MissilePass(WeakPtr<IGame> game, JRenderer *r)
        : RenderPass(r), game(game)
        {
            setBackgroundColor(Vector(0,0.2,0));
            enableClearColor(true);
        }
        
        void onFire(Ptr<IWeapon> weapon) {
            WeakPtr<IActor> weak_round = weapon->lastFiredRound();
            round = weak_round.lock();
            if (!round) return;
            
            Ptr<IGame> thegame = game.lock();
            if (!thegame) return;
            
            Ptr<FollowingCamera> cam = new FollowingCamera();
            cam->setTarget((Ptr<IMovementProvider>)round);
            cam->setFocus(1.5);
            cam->setAspect(1.0);
            cam->setNearDistance(5.0);
            cam->setFarDistance(thegame->getEnvironment()->getClipMax());
            
            RenderContext ctx(cam);
            ctx.draw_gunsight = false;
            ctx.draw_console = false;
            
            Ptr<SceneRenderPass> scene = new SceneRenderPass(thegame, ctx);
            
            stackedOn(scene);
            enableClearColor(false);
        }
        
        virtual void draw() {
            if (round && !round->isAlive()) {
                round = 0;
                stackedOn(0);
                enableClearColor(true);
            }
        }
    };
    
public:
    MissileViewModule(WeakPtr<IGame> game) : game(game) { }
    
    void addWeapon(Ptr<Weapon> w) {
        weapons.push_back(w);
        w->onFireSig().connect( SigC::slot(*this, &MissileViewModule::onFire) );
    }
    
    virtual Ptr<RenderPass> createRenderPass(JRenderer * renderer) {
        Ptr<IGame> thegame = game.lock();
        if (!thegame) return new RenderPass(renderer);

        Ptr<MissilePass> pass = new MissilePass(game, renderer);
        for(Weapons::iterator i=weapons.begin(); i!= weapons.end(); ++i) {
            (*i)->onFireSig().connect( SigC::slot(*pass, &MissilePass::onFire));
        }
        
        if (last_fired_weapon) {
            pass->onFire(last_fired_weapon);
        }

        Ptr<IFontMan> fontman = thegame->getFontMan();
        fontman->selectFont(thegame->getConfig()->query("MFD_font_title", "default"));
        
        Ptr<UI::Panel> panel = new UI::Panel(renderer);
        Ptr<UI::StaticLabel> label = new UI::StaticLabel("label", "MissileCam", fontman->getFont());
        label->setColor(Vector(0,1,0));
        label->setAlpha(1);
        panel->add(label, "root",
            UI::Panel::HCENTER|UI::Panel::VCENTER,
            UI::Panel::HCENTER|UI::Panel::VCENTER,
            Vector2(0,-0.2));
        
        Ptr<UI::PanelRenderPass> result = new UI::PanelRenderPass(renderer);
        result->setPanel(panel);
        result->stackedOn(pass);
        return result;

    }
    
    void onFire(Ptr<IWeapon> weapon) {
        last_fired_weapon = weapon;
    }
};

class DistanceLabel : public UI::Label {
    Ptr<Targeter> targeter;
public:
    std::string format;
    
    DistanceLabel(const char *name, Ptr<Targeter> targeter)
    : UI::Label(name), targeter(targeter), format("%.1f km")
    { }
    
    virtual std::string getText() {
        Ptr<IActor> target = targeter->getCurrentTarget();
        if (!target) return "";
        
        char buf[32];
        float dist = (target->getLocation()-targeter->getSubjectActor().getLocation()).length();
        sprintf(buf, format.c_str(), dist/1000);
        return buf;
    }
};

class AttitudeLabel : public UI::Label {
    Ptr<Targeter> targeter;
public:
    AttitudeLabel(const char *name, Ptr<Targeter> targeter)
    : UI::Label(name), targeter(targeter)
    { }
    
    virtual std::string getText() {
        Ptr<IActor> target = targeter->getCurrentTarget();
        if (!target) return "";
        if (!target->getFaction()) return "";
        
        setColor(target->getFaction()->getColor());
        
        switch(targeter->getSubjectActor().getFaction()->getAttitudeTowards(target->getFaction())) {
        case Faction::FRIENDLY:
            return "Friendly";
        case Faction::NEUTRAL:
            return "Neutral";
        case Faction::HOSTILE:
            return "Hostile";
        }
        
        // should never be reached
        return "attitude?";
    }
};

class NameLabel : public UI::Label {
    Ptr<Targeter> targeter;
public:
    NameLabel(const char *name, Ptr<Targeter> targeter)
    : UI::Label(name), targeter(targeter)
    { }
    
    virtual std::string getText() {
        Ptr<IActor> target = targeter->getCurrentTarget();
        if (!target) return "";
        Ptr<TargetInfo> ti = target->getTargetInfo();
        if (!ti) return "";
        
        return ti->getTargetName();
    }
};

class DisplayRangeLabel : public UI::Label {
    Ptr<Targeter> targeter;
public:
    std::string format;

    DisplayRangeLabel(const char *name, Ptr<Targeter> targeter)
    : UI::Label(name), targeter(targeter), format("%.0f")
    { }
    
    virtual std::string getText() {
        char buf[32];
        sprintf(buf, format.c_str(), targeter->getDisplayRange()/1000);
        return buf;
    }
};

    
class TargetViewModule : public MfdModule {
    WeakPtr<IGame> game;
    WeakPtr<Drone> drone;
    
    class TargetPass : public SceneRenderPass {
        WeakPtr<Drone> drone;
        Ptr<SimpleCamera> cam;
    public:
        TargetPass(Ptr<IGame> game, Ptr<Drone> drone, JRenderer *r)
        : SceneRenderPass(game, RenderContext(0)), cam(new SimpleCamera), drone(drone)
        {
            setBackgroundColor(Vector(0,0.2,0));
            enableClearColor(true);
            context.draw_gunsight = false;
            context.draw_console = false;
            
            // We initialized the render context's camera member to 0 and
            // SceneRenderPass'es constructor took that and set it as the target
            // for mirror_cam.
            // Let's fix both.
            
            context.camera = cam;
            if (mirror_cam) {
                mirror_cam->setTarget(cam);
            }
            
            preDepends().connect(SigC::slot(*this, &TargetPass::update));
        }
        
        void update(Ptr<RenderPass>) {
            Ptr<IGame> thegame = this->thegame.lock();
            Ptr<Drone> drone = this->drone.lock();
            if (!thegame || !drone) return;
            
            Ptr<IActor> target = drone->getTargeter()->getCurrentTarget();
            if (!target) return;
            
            Vector front = (target->getLocation() - drone->getLocation()).normalize();
            Vector right = (drone->getUpVector() % front).normalize();
            Vector up = front % right;
            cam->setOrientation(up,right,front);
            cam->setLocation(drone->getLocation());
            
            float target_size = 5.0f;
            if (target->getTargetInfo()) {
                target_size = target->getTargetInfo()->getTargetSize();
            }
            
            float focus = 0.25 * (drone->getLocation()-target->getLocation()).length() / target_size;
            cam->setFocus(std::max(1.0f, std::min(100.0f, focus)));
            cam->setAspect(1);
            cam->setNearDistance(6.0);
            cam->setFarDistance(thegame->getEnvironment()->getClipMax());
        }
        
        virtual void draw() {
            Ptr<Drone> drone = this->drone.lock();
            if (!drone) return;
            
            Ptr<IActor> target = drone->getTargeter()->getCurrentTarget();
            if (!target || !target->isAlive()) return;
            
            SceneRenderPass::draw();
        }
    };
    
public:
    TargetViewModule(WeakPtr<IGame> game, Ptr<Drone> drone) : game(game), drone(drone) { }
    
    virtual Ptr<RenderPass> createRenderPass(JRenderer * renderer) {
        Ptr<IGame> thegame = game.lock();
        Ptr<Drone> drone = this->drone.lock();
        if (!thegame || !drone) return new RenderPass(renderer);

        Ptr<TargetPass> pass = new TargetPass(thegame, drone, renderer);

        Ptr<IFontMan> fontman = thegame->getFontMan();
        fontman->selectFont(thegame->getConfig()->query("MFD_font_title", "default"));
        
        Ptr<UI::Panel> panel = new UI::Panel(renderer);
        Ptr<UI::StaticLabel> label = new UI::StaticLabel("label", "Target", fontman->getFont());
        label->setColor(Vector(0,1,0));
        label->setAlpha(1);
        panel->add(label, "root",
            UI::Panel::HCENTER|UI::Panel::VCENTER,
            UI::Panel::HCENTER|UI::Panel::VCENTER,
            Vector2(0,-0.2));
        
        fontman->selectFont(thegame->getConfig()->query("MFD_font_default", "default"));

        Ptr<NameLabel> name = new NameLabel("name", drone->getTargeter());
        name->setColor(Vector(0,1,0));
        name->setFont(fontman->getFont());
        name->setAlpha(1);
        panel->add(name, "label",
            UI::Panel::HCENTER|UI::Panel::BOTTOM,
            UI::Panel::HCENTER|UI::Panel::TOP);
        
        fontman->selectFont(thegame->getConfig()->query("MFD_font_small", "default"));

        Ptr<DistanceLabel> range = new DistanceLabel("range", drone->getTargeter());
        range->setFont(fontman->getFont());
        range->setColor(Vector(1,0,0));
        range->setAlpha(1);
        panel->add(range, "root",
            UI::Panel::HCENTER|UI::Panel::VCENTER,
            UI::Panel::HCENTER|UI::Panel::VCENTER,
            Vector2(0,0.2));
        
        Ptr<AttitudeLabel> attitude = new AttitudeLabel("attitude", drone->getTargeter());
        attitude->setFont(fontman->getFont());
        attitude->setAlpha(1);
        panel->add(attitude, "range",
            UI::Panel::HCENTER|UI::Panel::TOP,
            UI::Panel::HCENTER|UI::Panel::BOTTOM);

        Ptr<UI::PanelRenderPass> result = new UI::PanelRenderPass(renderer);
        result->setPanel(panel);
        result->stackedOn(pass);
        return result;

    }
};


class RadarModule : public MfdModule, public SigC::Object {
    WeakPtr<IGame> game;
    Ptr<Targeter> targeter;
    Ptr<IPositionProvider> subject, view;
    Ptr<Texture> map_compass, map_lines;
    jcamera_t cam;
    float display_range;
public:
    RadarModule(WeakPtr<IGame> game,
                Ptr<Targeter> targeter,
                Ptr<IPositionProvider> subject,
                Ptr<IPositionProvider> view)
        : game(game)
        , targeter(targeter)
        , subject(subject)
        , view(view)
        , display_range(targeter->getDisplayRange())
    {
        Ptr<IGame> thegame = game.lock();
        if (!thegame) {
            return;
        }
        Ptr<IConfig> cfg = thegame->getConfig();
        map_compass = thegame->getTexMan()->query(cfg->query("Map_compass_tex"));
        map_lines = thegame->getTexMan()->query(cfg->query("Map_lines_tex"));
        
        Ptr<SimpleCamera> cam = new SimpleCamera;
        cam->setLocation(Vector(0,cfg->queryFloat("Map_y",0.5f),cfg->queryFloat("Map_z",-1.5)));
        cam->pointTo(Vector(0,cfg->queryFloat("Map_py",0),0));
        cam->setNearDistance(0.01);
        cam->setFarDistance(100);
        cam->setAspect(1);
        cam->setFocus(cfg->queryFloat("Map_f",1.5f));
        
        JCamera jcam;
        cam->getCamera(&jcam);
        this->cam = jcam.cam;
    }
    
    virtual Ptr<RenderPass> createRenderPass(JRenderer * renderer) {
        if (!renderer)
        {
            throw std::invalid_argument("RadarModule: renderer is null");
        }

        Ptr<IGame> thegame = game.lock();
        if (!thegame) return new RenderPass(renderer);
        
        Ptr<RenderPass> radar = new RenderPass(renderer);
        radar->enableClearColor(true);
        radar->preDraw().connect(SigC::slot(*this, &RadarModule::draw));
        
        Ptr<UI::Panel> panel = new UI::Panel(renderer);
        Ptr<IFontMan> fontman = thegame->getFontMan();
        fontman->selectFont(thegame->getConfig()->query("MFD_font_small", "default"));

        Ptr<DistanceLabel> dist = new DistanceLabel("distance", targeter);
        dist->setFont(fontman->getFont());
        dist->setColor(Vector(1,0,0));
        dist->setAlpha(1);
        dist->format = "%.1f";
        panel->add(dist, "root",
            UI::Panel::RIGHT|UI::Panel::VCENTER,
            UI::Panel::RIGHT|UI::Panel::VCENTER,
            Vector2(-0.02,0.17));

        Ptr<DisplayRangeLabel> range = new DisplayRangeLabel("display-range", targeter);
        range->setFont(fontman->getFont());
        range->setColor(Vector(1,0,0));
        range->setAlpha(1);
        panel->add(range, "root",
            UI::Panel::RIGHT|UI::Panel::VCENTER,
            UI::Panel::RIGHT|UI::Panel::VCENTER,
            Vector2(-0.02,-0.14));

        Ptr<NameLabel> name = new NameLabel("name", targeter);
        name->setFont(fontman->getFont());
        name->setColor(Vector(1,0,0));
        name->setAlpha(1);
        panel->add(name, "root",
            UI::Panel::LEFT|UI::Panel::VCENTER,
            UI::Panel::LEFT|UI::Panel::VCENTER,
            Vector2(0.02,0.17));
        
        Ptr<UI::PanelRenderPass> result = new UI::PanelRenderPass(renderer);
        result->setPanel(panel);
        result->stackedOn(radar);
        return result;
    }
    
private:
    void draw(Ptr<RenderPass> pass) {
        JRenderer * renderer = pass->getRenderer();
        
        float view_azimuth = getAzimuth(view->getFrontVector());
        float subject_azimuth = getAzimuth(subject->getFrontVector());
        
        Matrix3 world_to_view = RotateYMatrix(-(3.141593f/2-view_azimuth));
        Matrix3 subject_to_view = RotateYMatrix(-(subject_azimuth-view_azimuth));
        
        renderer->setCamera(&cam);
        
        renderer->setCullMode(JR_CULLMODE_NO_CULLING);
        renderer->disableZBuffer();
        renderer->enableAlphaBlending();
        renderer->disableFog();
        renderer->setAlpha(1);
        renderer->setColor(Vector(1,1,1));
        
	    // Draw the compass
	    renderer->pushMatrix();
	    renderer->multMatrix(Matrix4::Hom(world_to_view));
	    renderer->enableTexturing();
	    renderer->setTexture(map_compass->getTxtid());
	    drawCenteredQuad(renderer, 1);
	    renderer->disableTexturing();
        renderer->popMatrix();

	    // Draw the lines
	    renderer->pushMatrix();
	    renderer->multMatrix(Matrix4::Hom(subject_to_view));
	    renderer->enableTexturing();
	    renderer->setTexture(map_lines->getTxtid());
	    drawCenteredQuad(renderer, 1);
	    renderer->disableTexturing();
        renderer->popMatrix();
	    
	    // Draw the targets
	    renderer->setBlendMode(JR_BLENDMODE_ADDITIVE);
	    renderer->pushMatrix();
	    renderer->multMatrix(Matrix4::Hom(world_to_view));
	    display_range = 0.5*display_range + 0.5*targeter->getDisplayRange();
	    drawTargets(renderer, display_range, 1.0f/32);
        renderer->popMatrix();
	    renderer->setBlendMode(JR_BLENDMODE_BLEND);
        
        renderer->enableFog();
        renderer->disableAlphaBlending();
        renderer->enableZBuffer();
    }
    
    static float getAzimuth(const Vector & v) { return atan2(v[2],v[0]); }
    static void drawCenteredQuad(JRenderer *r, float s) {
        r->begin(JR_DRAWMODE_QUADS);
        r->setUVW(Vector(0,0,0));
        (*r) << Vector(-s,0,-s);
        r->setUVW(Vector(1,0,0));
        (*r) << Vector(s,0,-s);
        r->setUVW(Vector(1,1,0));
        (*r) << Vector(s,0,s);
        r->setUVW(Vector(0,1,0));
        (*r) << Vector(-s,0,s);
        r->end();
    }
    
    void drawTargets(JRenderer *r, float range, float icon_size) {
        typedef std::vector<Ptr<IActor> > Targets;
        Targets targets;
        targeter->listTargets(targets);
        targets.push_back(&targeter->getSubjectActor());
        for(Targets::iterator i=targets.begin(); i!= targets.end(); ++i) {
            Ptr<IActor> target = *i;
            
            Ptr<TargetInfo> target_info = target->getTargetInfo();
            if (!target_info || !target_info->isA(TargetInfo::DETECTABLE)) {
                continue;
            }
            
            float dist = (target->getLocation() - subject->getLocation()).length();
            if (dist > 0.95*range) {
                continue;
            }
            
            float s = icon_size * log(target_info->getTargetSize() + 10) / log(10);
            r->setColor(target->getFaction()->getColor() + Vector(.2,.2,.2));
            if (target_info->isA(TargetInfo::GUIDED_MISSILE)) {
                r->setColor(Vector(1,1,0));
            }
            
            Vector p = (target->getLocation() - subject->getLocation()) / range;
            p[1] = 0;
            
            r->begin(JR_DRAWMODE_QUADS);
            (*r) << (p+Vector(-s,0,0));
            (*r) << (p+Vector(0,0,-s));
            (*r) << (p+Vector(s,0,0));
            (*r) << (p+Vector(0,0,s));
            r->end();
            
            if (target == targeter->getCurrentTarget()) {
                s *= 2;
                r->begin(JR_DRAWMODE_CONNECTED_LINES);
                (*r) << (p+Vector(-s,0,0));
                (*r) << (p+Vector(0,0,-s));
                (*r) << (p+Vector(s,0,0));
                (*r) << (p+Vector(0,0,s));
                (*r) << (p+Vector(-s,0,0));
                r->end();
            }
        }
    }
};

DroneCockpit::DroneCockpit( WeakPtr<IGame> game,
                            JRenderer *renderer,
                            Ptr<Drone> drone,
                            Ptr<Model> interior_model,
                            Ptr<Model> mfd_model )
    : thegame(game)
    , drone(drone)
    , interior_model(interior_model)
    , mfd_model(mfd_model)
    , current_mfd_module(0)
{
    if (!renderer) throw std::invalid_argument("DroneCockpit::DroneCockpit renderer is null");
    mfd_pass = new RenderPass(renderer);
    mfd_pass->enableClearDepth(false);
    
    Ptr<MissileViewModule> missile_view = new MissileViewModule(game);
    missile_view->addWeapon(drone->getArmament()->getWeapon("Sidewinder"));
    missile_view->addWeapon(drone->getArmament()->getWeapon("Hydra"));
    
    mfd_modules.push_back(new RadarModule(thegame, drone->getTargeter(), drone, drone));
    mfd_modules.push_back(new TargetViewModule(thegame, drone));
    //mfd_modules.push_back(new MagViewModule(thegame, drone));
    mfd_modules.push_back(missile_view);
    
    Ptr<MfdModule> module = mfd_modules[0];
    mfd_pass->stackedOn( module->createRenderPass(renderer) );
    
    mfd_tex = mfd_model->getDefaultObject()->getGroups().back()->mtl.tex;
    assert(!mfd_model->getDefaultObject()->getGroups().empty());
    assert(mfd_tex);
}

DroneCockpit::~DroneCockpit()
{
}

void DroneCockpit::switchMfdMode() {
    if (mfd_modules.empty()) { return; }
    
    ++current_mfd_module;
    
    if (current_mfd_module == mfd_modules.size()) {
        current_mfd_module = 0;
    }
    
    JRenderer * renderer = mfd_pass->getRenderer();
    Ptr<MfdModule> module = mfd_modules[current_mfd_module];
    mfd_pass->stackedOn( module->createRenderPass(renderer) );
}

Ptr<RenderPass> DroneCockpit::createRenderPass(Ptr<ICamera> cam) {
    JRenderer *r = mfd_pass->getRenderer();
    if (!r)
    {
        throw std::invalid_argument("DroneCockpit:: renderer is null");
    }
    Ptr<RenderPass> result = new RenderPass(r);
    Ptr<Drone> drone = this->drone.lock();
    if (!drone) return result;
    
    result->addDependency(mfd_pass, mfd_tex);
    result->postDraw().connect(
        SigC::bind(SigC::slot(*this, &DroneCockpit::drawCockpit), cam));
    
    return result;
}

void DroneCockpit::drawCockpit(Ptr<RenderPass> pass, Ptr<ICamera> cam) {
    Ptr<Drone> drone = this->drone.lock();
    if (!drone) return;
    
    JRenderer *renderer = pass->getRenderer();

    JCamera jcam;
    cam->getCamera(&jcam);
    renderer->setCamera(&jcam.cam);
    
    float old_near = renderer->getClipNear();
    float old_far = renderer->getClipFar();
    renderer->setClipRange(cam->getNearDistance(),
                           cam->getFarDistance());
                           
    Ptr<JDirectionalLight> sun = renderer->createDirectionalLight();
    sun->setColor(Vector(1,1,1) - 0.25*Vector(.97,.83,.74));
    sun->setEnabled(true);
    sun->setDirection(Vector(-0.9, 0.4, 0).normalize());
    
    renderer->enableSmoothShading();
    renderer->disableFog();

    renderer->setCullMode(JR_CULLMODE_CULL_NEGATIVE);
    renderer->setAlpha(1);
    renderer->setColor(Vector(1,1,1));
    
    Transform xform = drone->getTransform();
    
    renderer->enableLighting();
    interior_model->draw(*renderer, xform);
    renderer->disableLighting();

    mfd_model->draw(*renderer, xform);
    
    renderer->enableFog();
    renderer->setClipRange(old_near, old_far);

}

