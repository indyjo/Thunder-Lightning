#ifndef IVIEW_H
#define IVIEW_H

#include <interfaces/IPositionProvider.h>
#include <interfaces/IMovementProvider.h>

struct IActor;
struct IDrawable;

struct IView : virtual public IPositionProvider,
			   virtual public IMovementProvider
{
    virtual Ptr<IActor> getViewSubject()=0;
    virtual Ptr<IDrawable> getGunsight()=0;
    virtual void enable()=0;
    virtual void disable()=0;
    virtual bool isEnabled()=0;
};

#endif
