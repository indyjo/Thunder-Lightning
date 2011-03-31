#include <fstream>
#include <string>
#include <stdexcept>
#include <modules/texman/TextureManager.h>
#include <debug.h>
#include "LegacyFont.h"


LegacyFont::LegacyFont(JRenderer* renderer, Ptr<TextureManager> texman, std::string dir, std::string name) {
    std::string jftname = dir + "/" + name + ".jft";
    
    std::ifstream in(jftname.c_str());
    if(!in) {
        throw std::runtime_error("FontMan: Couldn't load jft file");
    }

    std::string sprname;
    in >> sprname;
    sprname = dir + "/" + sprname;
    
    if(!std::ifstream(sprname.c_str())) {
        throw std::runtime_error("FontMan: Couldn't load spr file");
    }
    
    tex = texman->query(sprname.c_str(), JR_HINT_GREYSCALE);
    this->renderer = renderer;
    
    int n;
    in >> n;
    
    for(int i=0; i<n; i++) {
        int j;
        in >> j;
        FontChar & c = chars[j];
        
        c.enabled = true;
        in >> c.dx >> c.dy >> c.tx1 >> c.ty1 >> c.tx2 >> c.ty2;
        c.dx = (c.tx2-c.tx1) * tex->getWidth();
        c.dy = (c.ty1-c.ty2) * tex->getHeight();
    }
}

float LegacyFont::getCharWidth(char c) {
    FontChar & fc = chars[c];
    return fc.enabled ? fc.dx : 0;
}

float LegacyFont::getMaxCharWidth() {
    return getCharWidth('W');
}

float LegacyFont::getLineHeight() {
    FontChar & c = chars[' '];
    return c.dy;
}

float LegacyFont::getCharAdvance(char c) {
    return getCharWidth(c);
}

void LegacyFont::getStringDims(const char *str, float *out_width, float *out_height) {
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

int LegacyFont::constrainString(const char *str, float max_width, bool partial) {
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

void LegacyFont::drawString(const char *str,
                               const Vector2 &pos,
                               const Vector3 &color,
                               float alpha,
                               int anchor,
                               float startofline_x,
                               Vector2 * out_pos
                              )
{
    // Glyph drawing uses a glyph's top left position as pivot, so we have to
    // adapt this in respect to anchor.
    
    Vector2 pen = pos;
    
    float offset_y=0;
    if (TOP == (anchor & VMASK)) {
    } else if (VCENTER == (anchor & VMASK)) {
        float box_w = 0, box_h = 0;
        getStringDims(str, &box_w, &box_h);
        offset_y = -box_h/2;
    } else if (BOTTOM == (anchor & VMASK)) {
        float box_w = 0, box_h = 0;
        getStringDims(str, &box_w, &box_h);
        offset_y = -box_h;
    } else throw std::invalid_argument("Bad anchor in drawString");


    renderer->disableSmoothShading();
    renderer->enableAlphaBlending();
    renderer->enableTexturing();
    renderer->setTexture(tex->getTxtid());
    
    renderer->setColor(color);
    renderer->setAlpha(alpha);

    while (*str) {
        std::string line;
        
        // We extract characters from str until we hit a null or newline
        while(*str && *str != '\n') {
            line += *str;
            ++str;
        }
        
        // We now measure the space that the extracted line would take
        float box_w = 0, box_h = 0;
        getStringDims(line.c_str(), &box_w, &box_h);
        
        // Take correct alignment into consideration
        float offset_x = 0;
        if (LEFT == (anchor & HMASK)) {
        } else if (HCENTER == (anchor & HMASK)) {
            offset_x = -box_w/2;
        } else if (RIGHT == (anchor & HMASK)) {
            offset_x = -box_w;
        } else throw std::invalid_argument("Bad anchor in drawString");

        // ... and draw the line at the computed location
        drawStringSimple(line.c_str(), pen + Vector2(offset_x, offset_y));
        pen[0] += box_w;

        // Now consider newlines and translate the pen position accordingly
        while (*str == '\n') {
            pen[0]  = startofline_x;
            pen[1] += getLineHeight();
            ++str;
        }
    }

    renderer->enableSmoothShading();
    renderer->disableAlphaBlending();
    renderer->disableTexturing();
    
    if (out_pos) {
        *out_pos = pen;
    }
}

void LegacyFont::drawStringSimple(const char *str,
                                     const Vector2 &pos
                                    )
{

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
}

