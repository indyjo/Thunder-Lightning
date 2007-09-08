#include <cmath>
#include <interfaces/IConfig.h>
#include <interfaces/IFont.h>
#include <interfaces/IFontMan.h>
#include <interfaces/IGame.h>
#include <modules/flight/flightinfo.h>
#include <modules/jogi/JRenderer.h>
#include <modules/environment/environment.h>
#include <DataNode.h>

#include "flight.h"

void FlexibleGunsight::addBasicCrosshairs(const char *parent) {
    addModule(new CrosshairModule(),
        parent, HCENTER | VCENTER, HCENTER | VCENTER);
}

void FlexibleGunsight::addFlightModules(Ptr<IGame> game, FlightInfo &fi, Ptr<DataNode> controls)
{
    Ptr<UI::Component> hud_frame = new HUDFrameModule();
    addModule(hud_frame, "screen", HCENTER | VCENTER, HCENTER | VCENTER);
    addModule(
        new HeightGraphModule(game,fi),
       "hud-frame", RIGHT | VCENTER, RIGHT | VCENTER,
        Vector(-8,0,0));
    addModule(
        new SpeedGraphModule(game,fi),
       "hud-frame", LEFT | VCENTER, LEFT | VCENTER,
        Vector(8,0,0));
    addModule(new HeightModule(game,fi),
       "height-graph", LEFT | VCENTER, RIGHT | VCENTER,
       Vector(-8,0,0));
    addModule(new SpeedModule(game,fi),
        "speed-graph", RIGHT | VCENTER, LEFT | VCENTER,
        Vector(8,0,0));
    addModule(
    	new HorizonIndicator(
    		game,fi, hud_frame->getWidth(), hud_frame->getHeight()),
    	"hud-frame", HCENTER | VCENTER, HCENTER | VCENTER);
    addModule(
        new GearHookIndicator( game, controls),
        "screen", RIGHT | BOTTOM, RIGHT | BOTTOM,
        Vector(-5,-5,0));
}



CrosshairModule::CrosshairModule()
: UI::Component("crosshairs", 33, 33)
{
}

void CrosshairModule::draw(UI::Panel & gunsight) {
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


HUDFrameModule::HUDFrameModule()
: UI::Component("hud-frame")
{
};

void HUDFrameModule::draw(UI::Panel & gunsight) {
	JRenderer *r = gunsight.getRenderer();
	UI::Surface surface = gunsight.getSurface();
	surface.translateOrigin(offset[0],offset[1]);
	
	r->pushMatrix();
	r->multMatrix(surface.getMatrix());
	
	r->setColor(Vector(0,1,0));
	r->begin(JR_DRAWMODE_CONNECTED_LINES);
	*r << Vector(8,1,0) << Vector(1,1,0)
	   << Vector(1,height-1,0) << Vector(8,height-1,0);
	r->end();
	r->begin(JR_DRAWMODE_CONNECTED_LINES);
	*r << Vector(width-1-8,0,0) << Vector(width-1,0,0)
	   << Vector(width-1,height-1,0) << Vector(width-1-8,height-1,0);
	r->end();
	
	r->popMatrix();
}

void HUDFrameModule::onLayout(UI::Panel & panel) {
    width = panel.getSurface().getWidth() / 2;
    height = panel.getSurface().getHeight() / 2;
}

SpeedModule::SpeedModule(Ptr<IGame> game, FlightInfo& fi)
:	UI::Label("speed", game->getFontMan()->selectNamedFont("HUD_font_big")),
	flight_info(fi)
{
    setColor(Vector(0,1,0));
}

std::string SpeedModule::getText() {
    char buf[16];
    snprintf(buf,16,"%.0f", flight_info.getCurrentSpeed() * 3.6f);
    return buf;
}

HeightModule::HeightModule(Ptr<IGame> game, FlightInfo& fi)
:	UI::Label("height", game->getFontMan()->selectNamedFont("HUD_font_big")),
	flight_info(fi)
{
    setColor(Vector(0,1,0));
}

std::string HeightModule::getText() {
    char buf[16];
    snprintf(buf,16,"%.0f", flight_info.getCurrentAltitude());
    return buf;
}

HeightGraphModule::HeightGraphModule(Ptr<IGame> game, FlightInfo & fi)
:   UI::Component("height-graph", 48, 16),
    flight_info(fi)
{
    fontman = game->getFontMan();
}

void HeightGraphModule::draw(UI::Panel& gunsight) {
    JRenderer *r = gunsight.getRenderer();
    UI::Surface surface = gunsight.getSurface();
    surface.translateOrigin(offset[0],offset[1]);

    r->pushMatrix();
    r->multMatrix(surface.getMatrix());
    
    fontman->selectNamedFont("HUD_font_small");
    fontman->setAlpha(1);
    fontman->setColor(Vector(0,1,0));
  
    r->setAlpha(1);
    r->setColor(Vector(0,1,0));
    r->begin(JR_DRAWMODE_LINES);
    *r << Vector(0, this->height/2, 0) << Vector(5, this->height/2, 0);
    r->end();
    
    float altitude = flight_info.getCurrentAltitude();
    
    float height_range = 250;
    float step_size = 50;
 
    float step_alt = std::max(0.0f, altitude - height_range/2);
    step_alt = std::floor(step_alt/step_size) * step_size;
    while (step_alt <= altitude + height_range/2) {
        
        float y_bottom =  this->height
            - (step_alt - altitude + height_range/2) * (this->height / height_range);
        float y_top   =  this->height
            - (step_alt + step_size - altitude + height_range/2) * (this->height / height_range);
        
        if (y_bottom < this->height) {
            r->begin(JR_DRAWMODE_LINES);
            *r  << Vector(5, y_bottom, 0)
                << Vector(10, y_bottom, 0);
            r->end();
            fontman->setCursor(
                Vector(12,y_bottom-4,0),
                Vector(1,0,0),
                Vector(0,1,0));
            char buf[16];
            snprintf(buf,16,"%.0f", step_alt);
            fontman->print(buf);
        }
        
        y_bottom = std::min(this->height-1, y_bottom);
        y_top = std::max(0.0f, y_top);
        
        r->begin(JR_DRAWMODE_LINES);
        *r  << Vector(5, y_top, 0)
            << Vector(5, y_bottom, 0);
        r->end();
        
        step_alt += step_size;
    }
    
    { // Ground altitude
        float ground_alt = altitude - flight_info.getCurrentHeight();
        float y =  this->height
            - (ground_alt - altitude + height_range/2) * (this->height / height_range);
        if (y < this->height && y >= 0) {
            r->begin(JR_DRAWMODE_LINES);
            *r  << Vector(0, y, 0) << Vector(10, y, 0);
            *r  << Vector(0, y+2, 0) << Vector(10, y+2, 0);
            r->end();
        }
    }
    
    
    r->popMatrix();
}

void HeightGraphModule::onLayout(UI::Panel & panel) {
    height = panel.getSurface().getHeight() / 2 - 16;
}


SpeedGraphModule::SpeedGraphModule(Ptr<IGame> game, FlightInfo & fi)
:   UI::Component("speed-graph", 48, 16),
    flight_info(fi)
{
    fontman = game->getFontMan();
}

void SpeedGraphModule::draw(UI::Panel& gunsight) {
    JRenderer *r = gunsight.getRenderer();
    UI::Surface surface = gunsight.getSurface();
    surface.translateOrigin(offset[0],offset[1]);

    r->pushMatrix();
    r->multMatrix(surface.getMatrix());
    
    fontman->selectNamedFont("HUD_font_small");
    fontman->setAlpha(1);
    fontman->setColor(Vector(0,1,0));
  
    r->setAlpha(1);
    r->setColor(Vector(0,1,0));
    r->begin(JR_DRAWMODE_LINES);
    *r  << Vector(this->width, this->height/2, 0)
        << Vector(this->width-5, this->height/2, 0);
    r->end();
    
    float speed = flight_info.getCurrentSpeed()*3.6f;
    
    float speed_range = 400;
    float step_size = 100;
    
    if (speed < 250) {
        speed_range=250;
        step_size = 50;
    } else if (speed > 1000) {
        speed_range = 1000;
        step_size = 250;
    }
 
    float step_speed = std::max(0.0f, speed - speed_range/2);
    step_speed = std::floor(step_speed/step_size) * step_size;
    while (step_speed <= speed + speed_range/2) {
        
        float y_bottom =  this->height
            - (step_speed - speed + speed_range/2) * (this->height / speed_range);
        float y_top   =  this->height
            - (step_speed + step_size - speed + speed_range/2) * (this->height / speed_range);
        
        if (y_bottom < this->height) {
            r->begin(JR_DRAWMODE_LINES);
            *r  << Vector(this->width-5, y_bottom, 0)
                << Vector(this->width-10, y_bottom, 0);
            r->end();
            fontman->setCursor(
                Vector(0,y_bottom-4,0),
                Vector(1,0,0),
                Vector(0,1,0));
            char buf[16];
            snprintf(buf,16,"%.0f", step_speed);
            fontman->print(buf);
        }
        
        y_bottom = std::min(this->height-1, y_bottom);
        y_top = std::max(0.0f, y_top);
        
        r->begin(JR_DRAWMODE_LINES);
        *r  << Vector(this->width-5, y_top, 0)
            << Vector(this->width-5, y_bottom, 0);
        r->end();
        
        step_speed += step_size;
    }
    
    r->popMatrix();
}

void SpeedGraphModule::onLayout(UI::Panel & panel) {
    height = panel.getSurface().getHeight() / 2 - 16;
}

HorizonIndicator::HorizonIndicator(
	Ptr<IGame> game,
	FlightInfo &fi,
	float w,
	float h)
:	UI::Component("horizon-indicator",w,h),
	fontman(game->getFontMan()),
	flight_info(fi)
{
}

#define PI 3.141593f

void HorizonIndicator::draw(UI::Panel & gunsight) {
	UI::Surface surf = gunsight.getSurface();
    surf.translateOrigin(offset[0],offset[1]);
	
	JRenderer *r = gunsight.getRenderer();
	r->pushMatrix();
	r->multMatrix(surf.getMatrix());
	r->setColor(Vector(0,1,0));
	
	Ptr<ICamera> cam = gunsight.getCamera();
	float pixels_per_degree = PI/180*surf.getHeight()*cam->getFocus()/2;
	float roll = flight_info.getCurrentRoll();
	float pitch = flight_info.getCurrentPitch();
	Vector right = Vector(std::cos(roll),-std::sin(roll),0);
	Vector up = Vector(-std::sin(roll), -std::cos(roll),0);
	for(int i=-80; i<= 80; i+=20) {
		float degrees = (float) i - pitch*180/PI;
		if (cos(degrees*PI/180) <= 0) continue;
		float elev = tan(degrees*PI/180) *surf.getHeight()/2*cam->getFocus();
		Vector center = Vector(width/2,height/2,0) + elev * up;
		drawIndicator(r,i,center,right);
	}
	
	r->popMatrix();
}

void HorizonIndicator::drawIndicator(
	JRenderer *r,
	int degrees,
	Vector center, Vector right)
{
	Vector up(right[1], -right[0], 0);
	r->begin(JR_DRAWMODE_LINES);
	*r << center-100*right-5*up << center-100*right;
	*r << center-100*right << center-50*right;
	*r << center+50*right << center+100*right;
	*r << center+100*right << center+100*right-5*up;
	r->end();
}

GearHookIndicator::GearHookIndicator(
	Ptr<IGame> game,
	Ptr<DataNode> controls)
:	UI::Component("horizon-indicator",150,40),
    controls(controls),
	fontman(game->getFontMan())
{
    Ptr<IFont> font = fontman->selectNamedFont("HUD_font_medium");
    font->getStringDims("Gear is DOWN\nHook is DOWN", &width, &height);
}

void GearHookIndicator::draw(UI::Panel & gunsight) {
    UI::Surface surf = gunsight.getSurface();
    surf.translateOrigin(offset[0],offset[1]);
    	
    fontman->selectNamedFont("HUD_font_medium");

    fontman->setCursor(
        surf.getOrigin(),
        surf.getDX(),
        surf.getDY());
    fontman->setAlpha(1);

    if(controls->getBool("landing_gear")) {
        fontman->setColor(Vector(1,1,0));
        fontman->print("Gear is DOWN\n");
    } else {
        fontman->setColor(Vector(0,0,1));
        fontman->print("Gear is UP\n");
    }

    if(controls->getBool("landing_hook")) {
        fontman->setColor(Vector(1,1,0));
        fontman->print("Hook is DOWN");
    } else {
        fontman->setColor(Vector(0,0,1));
        fontman->print("Hook is UP");
    }
}

