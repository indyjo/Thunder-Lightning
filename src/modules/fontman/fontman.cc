#include <fstream>
#include <stdexcept>
#include <cctype>
#include <cstdio>
#include <interfaces/IConfig.h>
#include <modules/math/SpecialMatrices.h>
#include "LegacyFont.h"
#include "fontman.h"


using namespace std;

namespace {
    typedef IFontMan::FontSpec::Style Style;
    
    Style pop_style(std::string & repr, 
                    Style default_style)
    {
        std::string::size_type pos = repr.find_last_of("-");
        if (pos == std::string::npos) {
            return default_style;
        }
        
        std:string style_str = repr.substr(pos+1);
        if (style_str == "default" || style_str == "standard" || style_str == "normal") {
            repr.erase(pos);
            return IFontMan::FontSpec::STANDARD;
        } else if (style_str == "bold") {
            repr.erase(pos);
            return IFontMan::FontSpec::BOLD;
        }
        
        return default_style;
    }
    
    int pop_size(std::string & repr,
                 int default_size)
    {
        std::string::size_type pos = repr.find_last_of("-");
        if (pos == std::string::npos) {
            return default_size;
        }
        
        for(std::string::size_type i=pos+1; i<repr.size(); ++i) {
            if (!isdigit(repr[i])) {
                return default_size;
            }
        }
        
        std::string num = repr.substr(pos+1);
        repr.erase(pos);
        
        return atoi(num.c_str());
    }
}

void IFontMan::FontSpec::fromString(std::string repr)
{
    style = pop_style(repr, STANDARD);
    size = pop_size(repr, 14);
    name = repr;
}

IFontMan::FontSpec::FontSpec(const string & name, int size, Style style)
: name(name), size(size), style(style)
{ }


FontMan::FontMan(IGame * game) {
    ls_message("Initializing FontMan... ");
    thegame=game;
    renderer = thegame->getRenderer();
    dir = thegame->getConfig()->query("FontMan_dir","/");
    
    std::string default_name =
        thegame->getConfig()->query("FontMan_default_font", "dejavu-sans-16-bold");
    ls_message("FontMan: default font is %s\n", default_name.c_str());
    selectFont( default_name );
    
    // see if everything went well
    if (!current_font) {
        FontSpec spec(default_name);
        ls_error("ERROR: Couldn't load the default font: %s size %d type %d\n",
                 spec.name.c_str(), spec.size, (int)spec.style);
        throw(std::runtime_error("Failed to load default font"));
    }
    
    default_font = current_font;
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

Ptr<IFont> FontMan::selectFont(const FontSpec & fs) {
    char namebuf[256];
    snprintf(namebuf, 256, "%s-%d-%s", fs.name.c_str(), fs.size,
            fs.style==FontSpec::BOLD?"bold":"standard");
    string name(namebuf);
    
    if (fonts.find(name)!=fonts.end()) {
        current_font = fonts[name];
        return current_font;
    }
    
    string jftname = dir + "/" + name + ".jft";
    if(!ifstream(jftname.c_str())) {
        ls_error("FontMan: Couldn't load jft file %s! Substituting default font.\n", jftname.c_str());
        current_font = default_font;
        return current_font;
    }
    
    current_font = new LegacyFont(renderer, thegame->getTexMan(), dir, name);
    fonts[name] = current_font;

    return current_font;
}

Ptr<IFont> FontMan::selectFont(Ptr<IFont> font) {
    current_font = font;
    if (!font) {
        current_font = default_font;
    }
    return current_font;
}

Ptr<IFont> FontMan::selectNamedFont(const char *fontname) {
    return selectFont(thegame->getConfig()->query(fontname, "default"));
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


