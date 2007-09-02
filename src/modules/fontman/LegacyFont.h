#ifndef TNL_LEGACY_FONT_H
#define TNL_LEGACY_FONT_H

#include <interfaces/IFont.h>
#include <modules/math/Vector.h>

class JRenderer;
class TextureManager;
class Texture;

class LegacyFont : public IFont {
    struct FontChar {
        inline FontChar() : enabled(false) { }
        bool enabled;
        float dx, dy;
        float tx1, ty1;
        float tx2, ty2;
    };

    JRenderer *renderer;
    Ptr<Texture> tex;
    FontChar chars[256];

public:
    LegacyFont(JRenderer* renderer, Ptr<TextureManager> texman, std::string dir, std::string name);
    
    virtual float getCharWidth(char c);
    virtual float getMaxCharWidth();
    virtual float getLineHeight();
    virtual float getCharAdvance(char c);
    virtual void getStringDims(const char *str, float *out_width, float *out_height);
    virtual int constrainString(const char *str, float max_width, bool partial);
    virtual void drawString(const char *str,
                            const Vector2 &pos,
                            const Vector3 &color,
                            float alpha,
                            int anchor,
                            float startofline_x,
                            Vector2 * out_pos
                            );
    void drawStringSimple(const char *str, const Vector2 &pos);
};


#endif

