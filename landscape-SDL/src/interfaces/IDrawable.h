#ifndef IDRAWABLE_H
#define IDRAWABLE_H

#include <object.h>

class IDrawable : virtual public Object
{
public:
    virtual void draw()=0;
};

#endif
