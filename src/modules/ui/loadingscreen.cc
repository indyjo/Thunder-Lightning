#include "loadingscreen.h"
#include <modules/environment/environment.h>
#include <interfaces/IFontMan.h>
#include <interfaces/IConfig.h>
#include "Surface.h"

#define MIN_PERCENT_DIFFERENCE 0.30

using namespace std;

LoadingScreen::LoadingScreen(IGame *thegame, const string & bgtex_name)
: thegame(thegame), last_status(0.0)
{
    ls_message("LoadingScreen: loading texture %s\n", bgtex_name.c_str());
    background = thegame->getTexMan()->
            query(bgtex_name.c_str(), JR_HINT_FULLOPACITY);
    ls_message("done\n");
    fontman = thegame->getFontMan();
}



void LoadingScreen::update(Status * stat) {
    double status = stat->getStatus();
    if (100.0*(status - last_status) < MIN_PERCENT_DIFFERENCE) return;
    last_status = status;
    JRenderer * r = thegame->getRenderer();
    UI::Surface surf = thegame->getScreenSurface();
    
    r->setCoordSystem(JR_CS_EYE);
    r->pushMatrix(),
    r->multMatrix(surf.getMatrix());
    
    r->enableTexturing();
    r->setTexture(background);
    r->enableSmoothShading();
    r->enableAlphaBlending();
    
    r->disableZBuffer();
    r->setClipRange(0.1, 10.0);
    
    r->setTexture(background);
    r->setColor(Vector(1,1,1));
    r->setAlpha(1.0);
        
    r->begin(JR_DRAWMODE_TRIANGLE_FAN);
    {
        r->setUVW(Vector(0,0,0));
        r->vertex(Vector(0,0,0));
        
        r->setUVW(Vector(1,0,0));
        r->vertex(Vector(surf.getWidth(),0,0));
        
        r->setUVW(Vector(1,1,0));
        r->vertex(Vector(surf.getWidth(), surf.getHeight(), 0));
        
        r->setUVW(Vector(0,1,0));
        r->vertex(Vector(0, surf.getHeight(), 0));
        
        r->setUVW(Vector(0,0,0));
        r->vertex(Vector(0,0,0));
    }
    r->end();
    
    fontman->selectFont(IFontMan::FontSpec("dungeon", 12));
    fontman->setColor(Vector(1,1,1));
    fontman->setAlpha(0.5);
    fontman->setCursor(
    	Vector(surf.getWidth()*256/1024, surf.getHeight()*700/768 - 30, 0),
    	Vector(1,0,0), Vector(0,1,0));
    fontman->print(stat->getDescription().c_str());
    
    r->popMatrix();

    surf.resize(1024, 768);
    surf.translateOrigin(256,700);
    r->pushMatrix();
    r->multMatrix(surf.getMatrix());
    
    r->enableAlphaBlending();
    r->disableTexturing();
    r->begin(JR_DRAWMODE_TRIANGLE_FAN);
    {
        r->setColor(Vector(1,1,1));
        r->setAlpha(0.5);
        *r << Vector(0,0,0) << Vector(512*status,0,0)
           << Vector(512*status,30,0) << Vector(0,30,0);
    }
    r->end();
    
    r->enableZBuffer();
    
    r->disableAlphaBlending();
    r->popMatrix();
    r->setCoordSystem(JR_CS_WORLD);
    
    Ptr<Environment> env = thegame->getEnvironment();
    r->setClipRange(env->getClipMin(), env->getClipMax());
    
    thegame->clearScreen();
}
