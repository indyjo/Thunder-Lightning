#include <interfaces/IActor.h>
#include <interfaces/IFontMan.h>
#include <interfaces/IFont.h>
#include <interfaces/IGame.h>
#include "gunsight.h"

class ControlModeModule : public GunsightModule {
	Ptr<IFont> font;
	Ptr<IActor> actor;
public:
	ControlModeModule(Ptr<IGame> game, Ptr<IActor> actor)
	    : GunsightModule("control-mode", 200, 20)
	    , actor(actor)
	{
        game->getFontMan()->selectFont(IFontMan::FontSpec("dungeon", 12, IFontMan::FontSpec::BOLD));
        font = game->getFontMan()->getFont();
	}
	
    void draw(FlexibleGunsight & gunsight) {
        UI::Surface surface = gunsight.getSurface();
        surface.translateOrigin(
		    offset[0]+width/2,
		    offset[1]+height);
        JRenderer * r = gunsight.getRenderer();
	    r->pushMatrix();
	    r->multMatrix(surface.getMatrix());
	    
        switch (actor->getControlMode()) {
        case IActor::UNCONTROLLED:
            font->drawString("UNCONTROLLED", Vector2(0), Vector(1,0,0), 1, IFont::BOTTOM|IFont::HCENTER);
            break;
        case IActor::AUTOMATIC:
            font->drawString("AUTOMATIC", Vector2(0), Vector(0,0,1), 1, IFont::BOTTOM|IFont::HCENTER);
            break;
        case IActor::MANUAL:
            font->drawString("MANUAL", Vector2(0), Vector(0,1,0), 1, IFont::BOTTOM|IFont::HCENTER);
            break;
        }
        
        r->popMatrix();
    }
};

void FlexibleGunsight::addBasics(Ptr<IGame> game, Ptr<IActor> actor)
{
    addModule(
        new ControlModeModule(game,actor),
       "screen", HCENTER | BOTTOM, HCENTER | BOTTOM);
}


