#ifndef IFONTMAN_H
#define IFONTMAN_H

#include <string>
#include <object.h>
#include <modules/math/Vector.h>

class IFontMan : virtual public Object
{
public:
    struct FontSpec {
        enum Style { STANDARD, BOLD };
        FontSpec(const std::string & name, int size = 10, Style style = STANDARD);

        std::string name;
        int size;
        Style style;
    };
    virtual void selectFont(const FontSpec & font)=0;
    virtual void setColor(const Vector & col)=0;
    virtual void setAlpha(float)=0;
    virtual void setCursor(const Vector & c,
                   const Vector & px,
                   const Vector & py)=0;
    virtual const Vector & getCursor()=0;
    virtual void print(const char *text)=0;
};

#endif
