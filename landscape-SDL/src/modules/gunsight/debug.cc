#include <string>
#include <interfaces/IActor.h>
#include <interfaces/IFontMan.h>
#include <interfaces/IGame.h>
#include <modules/jogi/JRenderer.h>
#include <modules/clock/clock.h>
#include <TargetInfo.h>
#include "debug.h"

void FlexibleGunsight::addDebugInfo(
	Ptr<IGame> game,
	Ptr<IActor> actor)
{
    addModule(new FPSModule(game),
        "screen", LEFT | TOP, LEFT | TOP, Vector(5,5,0));
    addModule(new TargetInfoModule(game,actor),
        "screen", LEFT | BOTTOM, LEFT | BOTTOM, Vector(5,-5,0));
}

FPSModule::FPSModule(Ptr<IGame> game)
:	GunsightModule("fps",50,12),game(game), fps(15)
{
}

void FPSModule::draw(FlexibleGunsight & gunsight) {
    if (!game->debugMode()) return;
	UI::Surface surface = gunsight.getSurface();
	Ptr<IFontMan> fontman = game->getFontMan();
	Ptr<Clock> clock = game->getClock();

	surface.translateOrigin(offset[0],offset[1]);
	
	fontman->selectFont(IFontMan::FontSpec(
		"dungeon", 12));
	
	fontman->setCursor(
		surface.getOrigin(),
		surface.getDX(),
		surface.getDY());
	fontman->setAlpha(1);
	fontman->setColor(Vector(0,1,0));
	
	char buf[16];
	fps = 0.9 * fps + 0.1 / clock->getRealFrameDelta();
	snprintf(buf,16,"%3.1f fps", fps);
	fontman->print(buf);
}


TargetInfoModule::TargetInfoModule(Ptr<IGame> game, Ptr<IActor> actor)
:	GunsightModule("target-info", 320, 200),
	game(game),
	actor(actor)
{
}

void TargetInfoModule::draw(FlexibleGunsight & gunsight) {
    if (!game->debugMode()) return;
	UI::Surface surface = gunsight.getSurface();
	JRenderer *r=game->getRenderer();

	surface.translateOrigin(offset[0],offset[1]);
	r->pushMatrix();
	r->multMatrix(surface.getMatrix());
	r->enableAlphaBlending();
	
	r->setColor(Vector(0,1,0));
	r->setAlpha(0.33);
	
	r->begin(JR_DRAWMODE_TRIANGLE_FAN);
	*r << Vector(0,0,0) << Vector(width,0,0)
	   << Vector(width, height,0) << Vector(0,height,0);
	r->end();
	
	r->setAlpha(1);
	r->begin(JR_DRAWMODE_CONNECTED_LINES);
	*r << Vector(0,0,0) << Vector(width-1,0,0)
	   << Vector(width-1, height-1,0) << Vector(0,height-1,0)
	   << Vector(0,0,0);
	r->end();
	
	Ptr<IFontMan> fontman = game->getFontMan();
	fontman->selectFont(IFontMan::FontSpec(
		"dungeon", 8));
	
	fontman->setCursor(
		Vector(5,5,0),
		Vector(1,0,0),
		Vector(0,1,0));
	fontman->setAlpha(1);
	fontman->setColor(Vector(1,1,1));
	fontman->print(actor->getTargetInfo()->getTargetInfo().c_str());
	
	r->disableAlphaBlending();
	r->popMatrix();
}
