#ifndef SIMPLE_VIEW_H
#define SIMPLE_VIEW_H

#include <landscape.h>
#include <interfaces/IView.h>

class FlexibleGunsight;

class SimpleView : virtual public IView {
public:
    SimpleView(Ptr<IActor> subject, Ptr<IDrawable> gunsight=0);
    
    // IPositionProvider
    virtual Vector getLocation();
    virtual Vector getFrontVector();
    virtual Vector getRightVector();
    virtual Vector getUpVector();
    virtual void getOrientation(Vector * up, Vector * right, Vector * front);
    
    // IMovementProvider
    // defaults to the subject's movement vector
    virtual Vector getMovementVector();
    
    // IView, trivial getter functions
    virtual Ptr<IActor> getViewSubject();
    virtual Ptr<IDrawable> getGunsight();
    
    virtual void enable();
    virtual void disable();    
    
protected:
	// To be overwritten by subclasses
	virtual void getPositionAndOrientation(
		Vector *pos, Matrix3 *orient)=0;
		
    Ptr<IActor> subject;
    Ptr<FlexibleGunsight> gunsight;
};

#endif
