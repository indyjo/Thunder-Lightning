#include <interfaces/IConfig.h>
#include <modules/actors/RelativeView.h>
#include <modules/engines/ObserverEngine.h>
#include <modules/gunsight/gunsight.h>
#include <modules/weaponsys/Targeter.h>
#include <remap.h>
#include <RenderPass.h>
#include "Observer.h"

Observer::Observer(Ptr<IGame> thegame)
    : SimpleActor(thegame)
    , dollying(false)
    , panning(false)
{
    mapEvents();
    setControlMode(MANUAL);
    setEngine(new ObserverEngine(thegame));
    setTargeter(new Targeter(*thegame,*this));
    thegame->getMainRenderPass()->preScene().connect(
        SigC::slot(*this, &Observer::update));
}

void Observer::action() {
}

void Observer::update(Ptr<RenderPass>) {
    Ptr<DataNode> c = getControls();
    Ptr<EventRemapper> e = thegame->getEventRemapper();
    
    c->setFloat("accel_x", 0);
    c->setFloat("accel_y", 0);
    c->setFloat("accel_z", 0);
    c->setFloat("rotate_x", 0);
    c->setFloat("rotate_y", 0);
    c->setFloat("rotate_z", 0);

    //FIXME: Should not use mouse_rel_* but proper axes to allow configurability
    //FIXME: Create a proper axis for mouse speed
    
    Ptr<IConfig> cfg = thegame->getConfig();
    float accel_x_factor = cfg->queryFloat("Observer_accel_x",  10);
    float accel_y_factor = cfg->queryFloat("Observer_accel_y", -10);
    float accel_z_factor = cfg->queryFloat("Observer_accel_z", -15);
    float rotate_x_factor = cfg->queryFloat("Observer_rotate_x", 0.2f);
    float rotate_y_factor = cfg->queryFloat("Observer_rotate_y", 0.2f);
    float global_factor = cfg->queryFloat("Observer_speed_factor", 0.01f);
    
    float factor = global_factor / thegame->getClock()->getRealFrameDelta();
    
    if (isDollying()) {
        c->setFloat("accel_z", factor*accel_z_factor*e->getAxis("mouse_rel_y"));
    } else if (isPanning()) {
        c->setFloat("accel_x", factor*accel_x_factor*e->getAxis("mouse_rel_x"));
        c->setFloat("accel_y", factor*accel_y_factor*e->getAxis("mouse_rel_y"));
    } else {
        c->setFloat("rotate_y", factor*rotate_y_factor*e->getAxis("mouse_rel_x"));
        c->setFloat("rotate_x", factor*rotate_x_factor*e->getAxis("mouse_rel_y"));
    }
    
    engine->run();
}

void Observer::draw() {
}


int Observer::getNumViews() {
	return 1;
}

Ptr<IView> Observer::getView(int n) {
    Ptr<FlexibleGunsight> gunsight = new FlexibleGunsight(thegame);
	gunsight->addDebugInfo(thegame, this);
	//gunsight->addTargeting(this, getTargeter());
    gunsight->addInfoMessage(thegame);

	return new RelativeView(
        this,
        Vector(0,0,0),
        Vector(1,0,0),
        Vector(0,1,0),
        Vector(0,0,1),
        gunsight);
}

void Observer::mapEvents() {
    Ptr<EventSheet> sheet = getEventSheet();
    sheet->map("+observer-dolly", SigC::bind(SigC::slot(*this, &Observer::setDollying), true));
    sheet->map("-observer-dolly", SigC::bind(SigC::slot(*this, &Observer::setDollying), false));
    sheet->map("+observer-pan", SigC::bind(SigC::slot(*this, &Observer::setPanning), true));
    sheet->map("-observer-pan", SigC::bind(SigC::slot(*this, &Observer::setPanning), false));
    sheet->map("observer-stop", SigC::slot(*this, &Observer::stop));
}

void Observer::setDollying(bool b) {
    dollying = b;
    if (!b) {
        //getControls()->setBool("stop_z", true);
    }
}

void Observer::setPanning(bool b) {
    panning = b;
    if (!b) {
        getControls()->setBool("stop_x", true);
        getControls()->setBool("stop_y", true);
    }
}

void Observer::stop() {
    getControls()->setBool("stop_z", true);
    getControls()->setBool("stop_x", true);
    getControls()->setBool("stop_y", true);
}

