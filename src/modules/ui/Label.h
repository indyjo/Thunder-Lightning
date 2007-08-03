#ifndef UI_LABEL_H
#define UI_LABEL_H

#include <string>
#include <modules/math/Vector.h>
#include "Component.h"

struct IFont;

namespace UI {

class Label : public Component {
    Ptr<IFont> font;
    Vector color, shadow_color;
    float alpha, shadow_alpha;
    bool draw_shadow;
public:
    inline Label(const char *name, Ptr<IFont> font = 0)
    : Component(name), font(font)
    , color(1,1,1), alpha(1)
    , shadow_color(0,0,0), shadow_alpha(0.5)
    , draw_shadow(true) { }
    
    inline Ptr<IFont> getFont() { return font; }
    inline void setFont(Ptr<IFont> f) { font = f; }
    
    virtual float getWidth();
    virtual float getHeight();
    
    inline const Vector & getColor() { return color; }
    inline void setColor(const Vector& c) { color = c; }
    
    inline float getAlpha() { return alpha; }
    inline void setAlpha(float a) { alpha = a; }
    
    inline void enableShadow(bool b) { draw_shadow = b; }
    inline bool isShadowEnabled() { return draw_shadow; }
    
    inline const Vector & getShadowColor() { return shadow_color; }
    inline void setShadowColor(const Vector& c) { shadow_color = c; }
    
    inline float getShadowAlpha() { return shadow_alpha; }
    inline void setShadowAlpha(float a) { shadow_alpha = a; }
    
    virtual void draw(Panel &);
    
    // child classes must override this
    virtual std::string getText()=0;
};

class StaticLabel : public Label {
    std::string text;
public:
    inline StaticLabel(const char *name, const std::string & text="", Ptr<IFont> font = 0)
    : Label(name, font), text(text) { }
    
    inline void setText(const std::string& t) { text=t; }
    virtual std::string getText();
};

} // namespace UI

#endif

