#include <fstream>
#include <cstdio>
#include "fontman.h"
#include <interfaces/IConfig.h>


using namespace std;

IFontMan::FontSpec::FontSpec(const string & name, int size, Style style)
: name(name), size(size), style(style)
{ }


FontMan::FontMan(IGame * game) {
    ls_message("Initializing FontMan... ");
    thegame=game;
    renderer = thegame->getRenderer();
    ls_message("(query)");
    dir = thegame->getConfig()->query("FontMan_dir","/");
    ls_message("(selectFont)");
    selectFont(FontSpec("arial"));
    ls_message("(setCursor)");
    setCursor(Vector(-1.0, 1.0, 2.0), Vector(0.001,0,0), Vector(0,-0.001,0));
    ls_message("(selectColor)");
    setColor(Vector(1,1,1));
    ls_message("(selectAlpha)");
    setAlpha(1.0);
    ls_message("done.\n");
}

FontMan::~FontMan()
{
    ls_message("~FontMan()\n");
    for(map<string, Font*>::iterator i=fonts.begin(); i!= fonts.end(); i++) {
        ls_message("Freeing font %s\n", i->first.c_str());
        delete i->second;
        ls_message("done.\n");
    }
    ls_message("~FontMan() done.\n");
}

void FontMan::selectFont(const FontSpec & fs) {
    char namebuf[256];
    snprintf(namebuf, 256, "%s-%d-%s", fs.name.c_str(), fs.size,
            fs.style==FontSpec::BOLD?"bold":"standard");
    string name(namebuf);
    
    if (fonts.find(name)!=fonts.end()) {
        current_font = fonts[name];
        return;
    }
    
    string jftname = dir + "/" + name + ".jft";
    string sprname;
    
    if(!ifstream(jftname.c_str())) {
        ls_error("FontMan: Couldn't load jft file %s!\n", jftname.c_str());
        return;
    }
    
    ifstream in(jftname.c_str());
    in >> sprname;
    
    sprname = dir + "/" + sprname;
    if(!ifstream(sprname.c_str())) {
        ls_error("FontMan: Couldn't load spr file %s!\n", sprname.c_str());
        return;
    }
    
    current_font = new Font;
    fonts[name] = current_font;
    current_font->tex =           // Load the font texture
            thegame->getTexMan()->query(sprname.c_str(), JR_HINT_GREYSCALE);
    
    int n;
    in >> n;
    
    for(int i=0; i<n; i++) {
        int j;
        in >> j;
        FontChar & c = current_font->chars[j];
        
        c.enabled = true;
        in >> c.dx >> c.dy >> c.tx1 >> c.ty1 >> c.tx2 >> c.ty2;
        c.dx = (c.tx2-c.tx1) * current_font->tex.getWidth();
        c.dy = (c.ty2-c.ty1) * current_font->tex.getHeight();
    }
}
    
void FontMan::setColor(const Vector & c) { color = c; }
void FontMan::setCursor(const Vector & cursor,
                        const Vector & px, const Vector & py) {
    this->cursor = cursor;
    begin = cursor;
    this->px = px;
    this->py = py;
}
void FontMan::setAlpha(float a) {
    alpha = a;
}

const Vector & FontMan::getCursor() { return cursor; }

void FontMan::print(const char *text) {
    renderer->disableSmoothShading();
    renderer->enableAlphaBlending();
    renderer->enableTexturing();
    renderer->setTexture(current_font->tex);
    
    while(*text) {
        unsigned char c = (unsigned char) *text;
        FontChar *fc = & current_font->chars[c];
        if (text[0]=='\r' && text[1]=='\n') text++;
        if (*text == '\r' || *text == '\n') {
            cursor = begin += py * current_font->chars['a'].dy;
        }
        
        if (!fc->enabled) {
            text++;
            continue;
        }
        
        renderer->begin(JR_DRAWMODE_TRIANGLE_FAN);
        {
            renderer->setColor(color);
            renderer->setAlpha(alpha);

            renderer->setUVW(Vector(fc->tx1, fc->ty1, 0));
            renderer->vertex(cursor);

            renderer->setUVW(Vector(fc->tx2, fc->ty1, 0));
            renderer->vertex(cursor + px*fc->dx);

            renderer->setUVW(Vector(fc->tx2, fc->ty2, 0));
            renderer->vertex(cursor + px*fc->dx + py*fc->dy);

            renderer->setUVW(Vector(fc->tx1, fc->ty2, 0));
            renderer->vertex(cursor + py*fc->dy);
        }
        renderer->end();
        
        cursor += fc->dx * px;
        text++;
    }
    renderer->enableSmoothShading();
    renderer->disableAlphaBlending();
    renderer->disableTexturing();
}
