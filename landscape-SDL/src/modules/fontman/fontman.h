#include <map>
#include <string>
#include <modules/math/Vector.h>
#include <landscape.h>
#include <interfaces/IFontMan.h>
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
    struct Font {
        TexPtr tex;
        FontChar chars[256];
    };
        
private:
    IGame * thegame;
    JRenderer * renderer;
    std::string dir;
    std::map<std::string, Font*> fonts;
    Font * current_font;
    Vector begin, cursor, px, py, color;
    float alpha;
};

