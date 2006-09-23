#include <stdexcept>
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
    current_font->renderer = renderer;
    
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

Ptr<IFontMetrics> FontMan::getMetrics() {
    return current_font;
}

Ptr<IFont> FontMan::getFont() {
    return current_font;
}

float FontMan::Font::getCharWidth(char c) {
    FontChar & fc = chars[c];
    return fc.enabled ? fc.dx : 0;
}

float FontMan::Font::getMaxCharWidth() {
    return getCharWidth('W');
}

float FontMan::Font::getLineHeight() {
    FontChar & c = chars['a'];
    return c.dy;
}

void FontMan::Font::getStringDims(const char *str, float *out_width, float *out_height) {
    int nlines = 1;
    float width = 0, linewidth = 0;
    for( ; *str; ++str) {
        if (*str == '\r') continue;
        if (*str == '\n') {
            linewidth = 0;
            nlines++;
            continue;
        }
        linewidth += getCharWidth(*str);
        if (linewidth > width) width = linewidth;
    }

    if (out_width) *out_width = width;
    if (out_height) *out_height = getLineHeight() * nlines;
}

int FontMan::Font::constrainString(const char *str, float max_width, bool partial) {
    float width = 0;
    int i = 0;
    for( ; str[i]; ++i) {
        FontChar & c = chars[str[i]];
        if (!c.enabled) continue;
        width += c.dx;
        if (width > max_width)
            return partial?i+1:i;
    }
    return i;
}

void FontMan::Font::drawString(const char *str, const Vector2 &pos, const Vector3 &color, float alpha, int anchor) {
    float box_w = 0, box_h = 0;
    getStringDims(str, &box_w, &box_h);

    Vector2 topleft(0,0);
    if (LEFT == (anchor & HMASK)) {
        topleft[0] = pos[0];
    } else if (HCENTER == (anchor & HMASK)) {
        topleft[0] = pos[0] - (box_w/2);
    } else if (RIGHT == (anchor & HMASK)) {
        topleft[0] = pos[0] - box_w;
    } else throw std::invalid_argument("Bad anchor in drawString");

    if (TOP == (anchor & VMASK)) {
        topleft[1] = pos[1];
    } else if (VCENTER == (anchor & VMASK)) {
        topleft[1] = pos[1] - (box_h/2);
    } else if (BOTTOM == (anchor & VMASK)) {
        topleft[1] = pos[1] - box_h;
    } else throw std::invalid_argument("Bad anchor in drawString");

    drawStringSimple(str, topleft, color, alpha);
}

void FontMan::Font::drawStringSimple(const char *str, const Vector2 &pos, const Vector3 &color, float alpha) {
    renderer->disableSmoothShading();
    renderer->enableAlphaBlending();
    renderer->enableTexturing();
    renderer->setTexture(tex);

    Vector2 start_of_line = pos;
    Vector2 cursor_pos = start_of_line;
    for ( ; *str; ++str) {
        char c = *str;
        if (c == '\n') {
            start_of_line += Vector2(0, getLineHeight());
            cursor_pos = start_of_line;
            continue;
        } else if ( c == '\r' ) {
            continue;
        }

        FontChar & fc = chars[c];
        if (fc.enabled) {
            renderer->begin(JR_DRAWMODE_TRIANGLE_FAN);
            {
                renderer->setColor(color);
                renderer->setAlpha(alpha);

                renderer->setUVW(Vector(fc.tx1, fc.ty1, 0));
                renderer->vertex(cursor_pos);

                renderer->setUVW(Vector(fc.tx2, fc.ty1, 0));
                renderer->vertex(cursor_pos + Vector2(fc.dx,0));

                renderer->setUVW(Vector(fc.tx2, fc.ty2, 0));
                renderer->vertex(cursor_pos + Vector2(fc.dx,0) + Vector2(0,fc.dy));

                renderer->setUVW(Vector(fc.tx1, fc.ty2, 0));
                renderer->vertex(cursor_pos + Vector2(0,fc.dy));
            }
            renderer->end();

            cursor_pos += Vector2(fc.dx,0);
        }
    }

    renderer->enableSmoothShading();
    renderer->disableAlphaBlending();
    renderer->disableTexturing();
}
