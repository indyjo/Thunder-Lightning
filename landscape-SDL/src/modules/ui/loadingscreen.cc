#include "loadingscreen.h"
#include <modules/environment/environment.h>
#include <interfaces/IFontMan.h>
#include <interfaces/IConfig.h>

#define MIN_PERCENT_DIFFERENCE 0.30

using namespace std;

LoadingScreen::LoadingScreen(IGame *thegame, const string & bgtex_name)
: thegame(thegame), last_status(0.0)
{
    ls_message("LoadingScreen: loading texture %s\n", bgtex_name.c_str());
    background = thegame->getTexMan()->
            query(bgtex_name.c_str(), JR_HINT_FULLOPACITY);
    fontman = thegame->getFontMan();
}



void LoadingScreen::update(Status * stat) {
    double status = stat->getStatus();
    if (100.0*(status - last_status) < MIN_PERCENT_DIFFERENCE) return;
    last_status = status;
    
    float aspect = 1.3;
    float focus = 1.0;
    Vector nw( -aspect,  1.0, focus);
    Vector se(  aspect, -1.0, focus);
    Vector dx( 2.0*aspect, 0, 0 );
    Vector dy( 0, -2.0, 0);
    Vector px = dx/1024;
    Vector py = dy/768;
    
    JRenderer * r = thegame->getRenderer();
    
    r->setCoordSystem(JR_CS_EYE);
    r->enableTexturing();
    r->setTexture(background);
    r->enableSmoothShading();
    r->enableAlphaBlending();
    
    r->disableZBuffer();
    r->setClipRange(0.1, 10.0);
    
    r->begin(JR_DRAWMODE_TRIANGLE_FAN);
    {
        r->setTexture(background);
        r->setColor(Vector(1,1,1));
        r->setAlpha(1.0);
        
        r->setUVW(Vector(0,0,0));
        r->vertex(nw);
        
        r->setUVW(Vector(1,0,0));
        r->vertex(nw + dx);
        
        r->setUVW(Vector(1,1,0));
        r->vertex(se);
        
        r->setUVW(Vector(0,1,0));
        r->vertex(nw + dy);
        
        r->setUVW(Vector(0,0,0));
        r->vertex(nw);
    }
    r->end();
    
    Vector o = nw + 256*px + 700*py;
    
    r->disableTexturing();
    r->begin(JR_DRAWMODE_TRIANGLE_FAN);
    {
        r->setColor(Vector(1,1,1));
        r->setAlpha(0.5);
        *r << o << o + 512*status*px << o + 512*status*px + 30*py
                << o + 30*py << o;
    }
    r->end();
    
    fontman->selectFont(IFontMan::FontSpec("dungeon", 12));
    fontman->setColor(Vector(1,1,1));
    fontman->setAlpha(0.5);
    fontman->setCursor( o - 20*py, px, py);
    fontman->print(stat->getDescription().c_str());
    
    r->enableZBuffer();
    
    r->disableAlphaBlending();
    r->setCoordSystem(JR_CS_WORLD);
    Ptr<Environment> env = thegame->getEnvironment();
    r->setClipRange(env->getClipMin(), env->getClipMax());
    
    thegame->clearScreen();
}
