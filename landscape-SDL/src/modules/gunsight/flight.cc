#include <cmath>
#include <interfaces/IFontMan.h>
#include <interfaces/IGame.h>
#include <modules/flight/flightinfo.h>
#include <modules/jogi/JRenderer.h>
#include <modules/environment/environment.h>


#include "flight.h"

void FlexibleGunsight::addBasicCrosshairs() {
    addModule(new CrosshairModule(),
        "screen", HCENTER | VCENTER, HCENTER | VCENTER);
}

void FlexibleGunsight::addFlightModules(Ptr<IGame> game, FlightInfo &fi)
{
    addModule(new HUDFrameModule(surface.getWidth(), surface.getHeight()),
        "screen", HCENTER | VCENTER, HCENTER | VCENTER);
    addModule(
        new HeightGraphModule(
                surface.getHeight(),game,fi),
       "hud-frame", RIGHT | VCENTER, RIGHT | VCENTER,
        Vector(-8,0,0));
    addModule(
        new SpeedGraphModule(
                surface.getHeight(),game,fi),
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
    		game,fi, surface.getWidth(), surface.getHeight()),
    	"screen", HCENTER | VCENTER, HCENTER | VCENTER);
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

HeightGraphModule::HeightGraphModule(
    float screen_h, Ptr<IGame> game, FlightInfo & fi)
:   GunsightModule("height-graph", 48, 0.5*screen_h - 16),
    flight_info(fi)
{
    fontman = game->getFontMan();
}

void HeightGraphModule::draw(FlexibleGunsight& gunsight) {
    JRenderer *r = gunsight.getRenderer();
    UI::Surface surface = gunsight.getSurface();
    surface.translateOrigin(offset[0],offset[1]);

    r->pushMatrix();
    r->multMatrix(surface.getMatrix());
    
    fontman->selectFont(IFontMan::FontSpec(
        "dungeon", 8));
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



SpeedGraphModule::SpeedGraphModule(
    float screen_h, Ptr<IGame> game, FlightInfo & fi)
:   GunsightModule("speed-graph", 48, 0.5*screen_h - 16),
    flight_info(fi)
{
    fontman = game->getFontMan();
}

void SpeedGraphModule::draw(FlexibleGunsight& gunsight) {
    JRenderer *r = gunsight.getRenderer();
    UI::Surface surface = gunsight.getSurface();
    surface.translateOrigin(offset[0],offset[1]);

    r->pushMatrix();
    r->multMatrix(surface.getMatrix());
    
    fontman->selectFont(IFontMan::FontSpec(
        "dungeon", 8));
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


HorizonIndicator::HorizonIndicator(
	Ptr<IGame> game,
	FlightInfo &fi,
	float w,
	float h)
:	GunsightModule("horizon-indicator",w,h),
	fontman(game->getFontMan()),
	flight_info(fi)
{
}

#define PI 3.141593f

void HorizonIndicator::draw(FlexibleGunsight & gunsight) {
	UI::Surface surf = gunsight.getSurface();
    surf.translateOrigin(offset[0],offset[1]);
	
	JRenderer *r = gunsight.getRenderer();
	r->pushMatrix();
	r->multMatrix(surf.getMatrix());
	r->setColor(Vector(0,1,0));
	
	Ptr<ICamera> cam = gunsight.getCamera();
	float pixels_per_degree = PI/180*surf.getHeight()/cam->getFocus();
	float roll = flight_info.getCurrentRoll();
	float pitch = flight_info.getCurrentPitch();
	Vector right = Vector(std::cos(roll),-std::sin(roll),0);
	Vector up = Vector(-std::sin(roll), -std::cos(roll),0);
	for(int i=-80; i<= 80; i+=20) {
		float degrees = (float) i - pitch*180/PI;
		Vector center = Vector(width/2,height/2,0) + 
			pixels_per_degree * degrees * up;
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
