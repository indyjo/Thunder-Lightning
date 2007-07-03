#ifndef RELATIVE_VIEW_H
#define RELATIVE_VIEW_H

#include <tnl.h>
#include <modules/actors/SimpleView.h>

class FlexibleGunsight;

class RelativeView : public SimpleView {
    Ptr<IPositionProvider> position;
    Ptr<IMovementProvider> movement;
    Vector p,up,right,front;
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

    virtual Vector getMovementVector();
    
protected:
	// Implements SimpleView
	virtual void getPositionAndOrientation(
		Vector *pos, Matrix3 *orient);

};


#endif
