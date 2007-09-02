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
    virtual Vector getCursor();
    virtual void print(const char *text);
    virtual Ptr<IFontMetrics> getMetrics();
    virtual Ptr<IFont> getFont();
    
private:
    Vector getTextWidth();
    Vector getTextHeight();
        
private:
    IGame * thegame;
    JRenderer * renderer;
    std::string dir;
    std::map<std::string, Ptr<IFont> > fonts;
    Ptr<IFont> current_font;
    Vector begin, px, py, color;
    Vector2 cursor;
    float alpha;
};

