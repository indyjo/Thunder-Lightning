#include <interfaces/IFont.h>
#include <modules/jogi/JRenderer.h>
#include "Panel.h"
#include "Surface.h"
#include "Label.h"

namespace UI {

float Label::getWidth() {
    float w,h;
    font->getStringDims(getText().c_str(), &w, &h);
    return w;
}

float Label::getHeight() {
    float w,h;
    font->getStringDims(getText().c_str(), &w, &h);
    if (isShadowEnabled()) ++h;
    return h;
}

void Label::draw(Panel& panel) {
    Surface surface = panel.getSurface();
    surface.translateOrigin(offset[0],offset[1]);
    
    JRenderer * r = panel.getRenderer();
    r->enableAlphaBlending();
    r->pushMatrix();
    r->multMatrix(surface.getMatrix());
    if (isShadowEnabled()) {
        font->drawString(getText().c_str(), Vector2(0,1), getShadowColor(), getShadowAlpha(), IFont::LEFT|IFont::TOP);
    }
    font->drawString(getText().c_str(), Vector2(0), getColor(), getAlpha(), IFont::LEFT|IFont::TOP);
    r->popMatrix();
    r->disableAlphaBlending();
}

std::string StaticLabel::getText() { return text; }

} // namespace UI
