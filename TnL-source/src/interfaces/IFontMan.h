#ifndef IFONTMAN_H
#define IFONTMAN_H

#include <string>
#include <object.h>
#include <modules/math/Vector.h>

#include "IFont.h"

struct IFontMan : virtual public Object
{
public:
    struct FontSpec {
        enum Style { STANDARD, BOLD };
        inline FontSpec(const char * str) { fromString(str); }
        inline FontSpec(const std::string & str) { fromString(str); }
        FontSpec(const std::string & name, int size, Style style = STANDARD);

        std::string name;
        int size;
        Style style;
        
        void fromString(std::string repr);
    };
    virtual Ptr<IFont> selectFont(const FontSpec & font)=0;
    virtual Ptr<IFont> selectFont(Ptr<IFont> font)=0;
    virtual Ptr<IFont> selectNamedFont(const char *)=0;
    virtual void setColor(const Vector & col)=0;
    virtual void setAlpha(float)=0;
    virtual void setCursor(const Vector & c,
                   const Vector & px,
                   const Vector & py)=0;
    virtual Vector getCursor()=0;
    virtual void print(const char *text)=0;

    /// Returns a metrics object describing the metrics of the currently selected font.
    /// Metrics are not bound to the fontman, i.e. even if another font is selected,
    /// the metrics describe the originally selected font.
    virtual Ptr<IFontMetrics> getMetrics()=0;

    /// Returns an object that draws text in font selected at the time of the method call.
    /// Just like the metrics object, selecting another font in the font manager
    /// does not alter the IFont object.
    virtual Ptr<IFont> getFont()=0;
};

#endif
