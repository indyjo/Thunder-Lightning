#ifndef INTERFACE_IMAP_H
#define INTERFACE_IMAP_H

#include <interfaces/IDrawable.h>

struct IMap: public IDrawable
{
public:
    virtual void enable()=0;
    virtual void disable()=0;

    virtual void magnify()=0;
    virtual void demagnify()=0;
};


#endif
