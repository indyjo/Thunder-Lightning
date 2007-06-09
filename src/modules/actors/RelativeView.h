#ifndef RELATIVE_VIEW_H
#define RELATIVE_VIEW_H

#include <tnl.h>
#include <interfaces/IView.h>

class FlexibleGunsight;

class RelativeView : virtual public IView {
    Ptr<IActor> subject;
    Ptr<IPositionProvider> position;
    Ptr<IMovementProvider> movement;
    Vector p,up,right,front;
    Ptr<FlexibleGunsight> gunsight;
public:
    RelativeView(Ptr<IActor> subject,
        const Vector & p,
        const Vector & right,
        const Vector & up,
        const Vector & front,
        Ptr<IDrawable> gunsight=0);
    RelativeView(Ptr<IPositionProvider> position, Ptr<IMovementProvider> movement,
        Ptr<IActor> subject, Ptr<IDrawable> gunsight=0);

    void setViewOffset(Vector p, Vector right, Vector up, Vector front);
    
    // IPositionProvider
    virtual Vector getLocation();
    virtual Vector getFrontVector();
    virtual Vector getRightVector();
    virtual Vector getUpVector();
    virtual void getOrientation(Vector * up, Vector * right, Vector * front);
    
    // IMovementProvider
    virtual Vector getMovementVector();
    
    // IView
    virtual Ptr<IActor> getViewSubject();
    virtual Ptr<IDrawable> getGunsight();
    virtual void enable();
    virtual void disable();    
};


#endif
