#include <interfaces/IFontMan.h>
#include <interfaces/IGame.h>
#include <modules/flight/flightinfo.h>
#include <modules/jogi/JRenderer.h>
#include <modules/environment/environment.h>


#include "FlightGunsight.h"


FlightGunsight::FlightGunsight(Ptr<IGame> game, FlightInfo &fi)
:	FlexibleGunsight(game)
{
	addModule(new CrosshairModule(),
		"screen", HCENTER | VCENTER, HCENTER | VCENTER);
	addModule(new HUDFrameModule(surface.getWidth(), surface.getHeight()),
		"screen", HCENTER | VCENTER, HCENTER | VCENTER);
	addModule(new SpeedModule(game,fi),
		"hud-frame", LEFT | VCENTER, LEFT | VCENTER,
		Vector(32,0,0));
	addModule(new HeightModule(game,fi),
		"hud-frame", RIGHT | VCENTER, RIGHT | VCENTER,
		Vector(-32,0,0));
}



CrosshairModule::CrosshairModule()
: GunsightModule("crosshairs", 33, 33)
{
}

void CrosshairModule::draw(FlexibleGunsight & gunsight) {
	JRenderer *r = gunsight.getRenderer();
	UI::Surface surface = gunsight.getSurface();
	surface.translateOrigin(
		offset[0]+width/2,
		offset[1]+height/2);
	
	r->pushMatrix();
	r->multMatrix(surface.getMatrix());
	
	r->begin(JR_DRAWMODE_LINES);
	
	r->setColor(Vector(0,1,0));
	*r << Vector(0,16,0)  << Vector(0,4,0);
	*r << Vector(16,0,0)  << Vector(4,0,0);
	*r << Vector(0,-16,0) << Vector(0,-4,0);
	*r << Vector(-16,0,0) << Vector(-4,0,0);
	
	r->end();
	
	r->popMatrix();
}


HUDFrameModule::HUDFrameModule(float screen_x, float screen_y)
: GunsightModule("hud-frame", 0.5*screen_x, 0.5*screen_y)
{
};

void HUDFrameModule::draw(FlexibleGunsight & gunsight) {
	JRenderer *r = gunsight.getRenderer();
	UI::Surface surface = gunsight.getSurface();
	surface.translateOrigin(offset[0],offset[1]);
	
	r->pushMatrix();
	r->multMatrix(surface.getMatrix());
	
	r->setColor(Vector(0,1,0));
	r->begin(JR_DRAWMODE_CONNECTED_LINES);
	*r << Vector(8,0,0) << Vector(0,0,0)
	   << Vector(0,height-1,0) << Vector(8,height-1,0);
	r->end();
	r->begin(JR_DRAWMODE_CONNECTED_LINES);
	*r << Vector(width-1-8,0,0) << Vector(width-1,0,0)
	   << Vector(width-1,height-1,0) << Vector(width-1-8,height-1,0);
	r->end();
	
	r->popMatrix();
}

SpeedModule::SpeedModule(Ptr<IGame> game, FlightInfo& fi)
:	GunsightModule("speed", 48, 16),
	flight_info(fi)
{
	fontman = game->getFontMan();
}

void SpeedModule::draw(FlexibleGunsight& gunsight) {
	UI::Surface surface = gunsight.getSurface();
	surface.translateOrigin(offset[0],offset[1]);
	
	fontman->selectFont(IFontMan::FontSpec(
		"dungeon", 12, IFontMan::FontSpec::BOLD));
	
	fontman->setCursor(
		surface.getOrigin(),
		surface.getDX(),
		surface.getDY());
	fontman->setAlpha(1);
	fontman->setColor(Vector(0,1,0));
	
	char buf[16];
	snprintf(buf,16,"%.0f",
		flight_info.getCurrentSpeed() * 3.6f);
	fontman->print(buf);
	
}

HeightModule::HeightModule(Ptr<IGame> game, FlightInfo& fi)
:	GunsightModule("height", 48, 16),
	flight_info(fi)
{
	fontman = game->getFontMan();
}

void HeightModule::draw(FlexibleGunsight& gunsight) {
	UI::Surface surface = gunsight.getSurface();
	surface.translateOrigin(offset[0],offset[1]);
	
	fontman->selectFont(IFontMan::FontSpec(
		"dungeon", 12, IFontMan::FontSpec::BOLD));
	
	fontman->setCursor(
		surface.getOrigin(),
		surface.getDX(),
		surface.getDY());
	fontman->setAlpha(1);
	fontman->setColor(Vector(0,1,0));
	
	char buf[16];
	snprintf(buf,16,"%.0f",flight_info.getCurrentHeight());
	fontman->print(buf);
	
}
