#include <fstream>
#include <cstdio>
#include <interfaces/IConfig.h>
#include <modules/math/SpecialMatrices.h>
#include "LegacyFont.h"
#include "fontman.h"


using namespace std;

IFontMan::FontSpec::FontSpec(const string & name, int size, Style style)
: name(name), size(size), style(style)
{ }


FontMan::FontMan(IGame * game) {
    ls_message("Initializing FontMan... ");
    thegame=game;
    renderer = thegame->getRenderer();
    dir = thegame->getConfig()->query("FontMan_dir","/");
    selectFont(FontSpec("arial"));
    setCursor(Vector(-1.0, 1.0, 2.0), Vector(0.001,0,0), Vector(0,-0.001,0));
    setColor(Vector(1,1,1));
    setAlpha(1.0);
    ls_message("done.\n");
}

FontMan::~FontMan()
{
    ls_message("~FontMan()\n");
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
    if(!ifstream(jftname.c_str())) {
        ls_error("FontMan: Couldn't load jft file %s!\n", jftname.c_str());
        return;
    }
    
    current_font = new LegacyFont(renderer, thegame->getTexMan(), dir, name);
    fonts[name] = current_font;
}
    
void FontMan::setColor(const Vector & c) { color = c; }
void FontMan::setCursor(const Vector & cursor,
                        const Vector & px, const Vector & py) {
    this->begin = cursor;
    this->cursor = Vector2(0,0);
    this->px = px;
    this->py = py;
}
void FontMan::setAlpha(float a) {
    alpha = a;
}

Vector FontMan::getCursor() { return begin + cursor[0]*px + cursor[1]*py; }

void FontMan::print(const char *text) {
    Vector pz = px % py;
    Matrix3 M = MatrixFromColumns(px,py,pz);
    Matrix M_hom = Matrix::Hom(M, begin);
    
    renderer->pushMatrix();
    renderer->multMatrix(M_hom);
    
    current_font->drawString(text, cursor, color, alpha,
        IFont::TOP | IFont::LEFT,
        0,
        &cursor);
    
    renderer->popMatrix();
}

Ptr<IFontMetrics> FontMan::getMetrics() {
    return current_font;
}

Ptr<IFont> FontMan::getFont() {
    return current_font;
}


