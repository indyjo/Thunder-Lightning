#ifndef RELATIVE_VIEW_H
#define RELATIVE_VIEW_H

#include <landscape.h>
#include <interfaces/IView.h>

class RelativeView : virtual public IView {
    Ptr<IActor> subject;
    Vector p,up,right,front;
    Ptr<IDrawable> gunsight;
public:
    RelativeView(Ptr<IActor> subject,
        const Vector & p,
        const Vector & right,
        const Vector & up,
        const Vector & front,
        Ptr<IDrawable> gunsight=0);
    
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
};


#endif
