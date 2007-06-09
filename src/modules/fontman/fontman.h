#include <map>
#include <string>
#include <modules/math/Vector.h>
#include <tnl.h>
#include <interfaces/IFontMan.h>
#include <interfaces/IFont.h>
#include <modules/texman/TextureManager.h>
#include <interfaces/IGame.h>

class FontMan : virtual public Object, public IFontMan {
public:
    FontMan(IGame * thegame);
    ~FontMan();
    
    virtual void selectFont(const FontSpec & font);
    virtual void setColor(const Vector & col);
    virtual void setAlpha(float alpha);
    virtual void setCursor(const Vector & c,
                   const Vector & px,
                   const Vector & py);
    virtual const Vector & getCursor();
    virtual void print(const char *text);
    virtual Ptr<IFontMetrics> getMetrics();
    virtual Ptr<IFont> getFont();
    
private:
    Vector getTextWidth();
    Vector getTextHeight();
        
    struct FontChar {
        inline FontChar() : enabled(false) { }
        bool enabled;
        float dx, dy;
        float tx1, ty1;
        float tx2, ty2;
    };
    
    struct Font : public IFont {
        JRenderer *renderer;
        TexPtr tex;
        FontChar chars[256];

        virtual float getCharWidth(char c);
        virtual float getMaxCharWidth();
        virtual float getLineHeight();
        virtual void getStringDims(const char *str, float *out_width, float *out_height);
        virtual int constrainString(const char *str, float max_width, bool partial);
        virtual void drawString(const char *str, const Vector2 &pos, const Vector3 &color, float alpha, int anchor);
        void drawStringSimple(const char *str, const Vector2 &pos, const Vector3 &color, float alpha);
    };
        
private:
    IGame * thegame;
    JRenderer * renderer;
    std::string dir;
    std::map<std::string, Ptr<Font> > fonts;
    Ptr<Font> current_font;
    Vector begin, cursor, px, py, color;
    float alpha;
};

