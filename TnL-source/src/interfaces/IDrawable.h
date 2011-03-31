#ifndef IDRAWABLE_H
#define IDRAWABLE_H

#include <object.h>

struct IDrawable : virtual public Object
{
public:
    virtual void draw()=0;
};

#endif
