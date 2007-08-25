#include <sigc++/sigc++.h>
#include <interfaces/ICamera.h>
#include <interfaces/IFont.h>
#include <interfaces/IFontMan.h>
#include <modules/camera/FollowingCamera.h>
#include <modules/camera/SimpleCamera.h>
#include <modules/environment/environment.h>
#include <modules/math/Transform.h>
#include <modules/model/model.h>
#include <modules/texman/Texture.h>
#include <modules/ui/Label.h>
#include <modules/ui/PanelRenderPass.h>
#include <modules/weaponsys/Armament.h>
#include <modules/weaponsys/Targeter.h>
#include <modules/weaponsys/Weapon.h>
#include <SceneRenderPass.h>

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
        fontman->selectFont(IFontMan::FontSpec("dungeon", 12, IFontMan::FontSpec::STANDARD));
        
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
        fontman->selectFont(IFontMan::FontSpec("dungeon", 12, IFontMan::FontSpec::STANDARD));
        
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

class MissileViewModule : public MfdModule {
    typedef std::vector<Ptr<Weapon> > Weapons;
    Weapons weapons;
    WeakPtr<IGame> game;
    
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
    }
    
    virtual Ptr<RenderPass> createRenderPass(JRenderer * renderer) {
        Ptr<IGame> thegame = game.lock();
        if (!thegame) return new RenderPass(renderer);

        Ptr<MissilePass> pass = new MissilePass(game, renderer);
        for(Weapons::iterator i=weapons.begin(); i!= weapons.end(); ++i) {
            (*i)->onFireSig().connect( SigC::slot(*pass, &MissilePass::onFire));
        }

        Ptr<IFontMan> fontman = thegame->getFontMan();
        fontman->selectFont(IFontMan::FontSpec("dungeon", 12, IFontMan::FontSpec::STANDARD));
        
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
    
    class RangeLabel : public UI::Label {
        WeakPtr<Drone> drone;
    public:
        RangeLabel(const char *name, Ptr<Drone> drone)
        : UI::Label(name), drone(drone)
        { }
        
        virtual std::string getText() {
            Ptr<Drone> drone = this->drone.lock();
            if (!drone) return "<no drone>";
            Ptr<IActor> target = drone->getTargeter()->getCurrentTarget();
            if (!target) return "";
            
            char buf[32];
            sprintf(buf, "%.1f km", (target->getLocation()-drone->getLocation()).length()/1000);
            return buf;
        }
    };
    
    class AttitudeLabel : public UI::Label {
        WeakPtr<SimpleActor> subject;
    public:
        AttitudeLabel(const char *name, Ptr<SimpleActor> subject)
        : UI::Label(name), subject(subject)
        { }
        
        virtual std::string getText() {
            Ptr<SimpleActor> subject = this->subject.lock();
            if (!subject) return "<no subject>";
            Ptr<IActor> target = subject->getTargeter()->getCurrentTarget();
            if (!target) return "";
            if (!target->getFaction()) return "";
            
            setColor(target->getFaction()->getColor());
            
            switch(subject->getFaction()->getAttitudeTowards(target->getFaction())) {
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
        WeakPtr<SimpleActor> subject;
    public:
        NameLabel(const char *name, Ptr<SimpleActor> subject)
        : UI::Label(name), subject(subject)
        { }
        
        virtual std::string getText() {
            Ptr<SimpleActor> subject = this->subject.lock();
            if (!subject) return "<no subject>";
            Ptr<IActor> target = subject->getTargeter()->getCurrentTarget();
            if (!target) return "";
            Ptr<TargetInfo> ti = target->getTargetInfo();
            if (!ti) return "";
            
            return ti->getTargetName();
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
        fontman->selectFont(IFontMan::FontSpec("dungeon", 12, IFontMan::FontSpec::STANDARD));
        
        Ptr<UI::Panel> panel = new UI::Panel(renderer);
        Ptr<UI::StaticLabel> label = new UI::StaticLabel("label", "Target", fontman->getFont());
        label->setColor(Vector(0,1,0));
        label->setAlpha(1);
        panel->add(label, "root",
            UI::Panel::HCENTER|UI::Panel::VCENTER,
            UI::Panel::HCENTER|UI::Panel::VCENTER,
            Vector2(0,-0.2));
        
        fontman->selectFont(IFontMan::FontSpec("dungeon", 10, IFontMan::FontSpec::BOLD));

        Ptr<NameLabel> name = new NameLabel("name", drone);
        name->setColor(Vector(0,1,0));
        name->setFont(fontman->getFont());
        name->setAlpha(1);
        panel->add(name, "label",
            UI::Panel::HCENTER|UI::Panel::BOTTOM,
            UI::Panel::HCENTER|UI::Panel::TOP);
        
        fontman->selectFont(IFontMan::FontSpec("dungeon", 8, IFontMan::FontSpec::BOLD));

        Ptr<RangeLabel> range = new RangeLabel("range", drone);
        range->setFont(fontman->getFont());
        range->setColor(Vector(1,0,0));
        range->setAlpha(1);
        panel->add(range, "root",
            UI::Panel::HCENTER|UI::Panel::VCENTER,
            UI::Panel::HCENTER|UI::Panel::VCENTER,
            Vector2(0,0.2));
        
        Ptr<AttitudeLabel> attitude = new AttitudeLabel("attitude", drone);
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
    mfd_pass = new RenderPass(renderer);
    mfd_pass->enableClearDepth(false);
    
    Ptr<MissileViewModule> missile_view = new MissileViewModule(game);
    missile_view->addWeapon(drone->getArmament()->getWeapon("Sidewinder"));
    missile_view->addWeapon(drone->getArmament()->getWeapon("Hydra"));
    
    mfd_modules.push_back(new TargetViewModule(thegame, drone));
    //mfd_modules.push_back(new MagViewModule(thegame, drone));
    mfd_modules.push_back(missile_view);
    
    Ptr<MfdModule> module = mfd_modules[0];
    mfd_pass->stackedOn( module->createRenderPass(renderer) );
    
    mfd_tex = mfd_model->getDefaultObject()->getGroups().back()->mtl.tex;
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
    Ptr<RenderPass> result = new RenderPass(mfd_pass->getRenderer());
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

