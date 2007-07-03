#include <interfaces/IActor.h>
#include <modules/gunsight/gunsight.h>
#include "RelativeView.h"

RelativeView::RelativeView(
	Ptr<IActor> subject,
    const Vector & p,
    const Vector & right,
    const Vector & up,
    const Vector & front,
    Ptr<IDrawable> gunsight)
:   SimpleView(subject, gunsight),
    position(subject),
    movement(subject),
    p(p),
    up(up), right(right), front(front)
{ }

RelativeView::RelativeView(Ptr<IPositionProvider> position,
                           Ptr<IMovementProvider> movement,
                           Ptr<IActor> subject,
                           Ptr<IDrawable> gunsight)
:   SimpleView(subject, gunsight),
    position(position),
    movement(movement),
    p(0,0,0),
    up(0,1,0),right(1,0,0),front(0,0,1)
{ }

void RelativeView::setViewOffset(Vector p, Vector right, Vector up, Vector front) {
    this->p = p;
    this->up = up;
    this->right = right;
    this->front = front;
}

Vector RelativeView::getMovementVector() {
    return movement->getMovementVector();
}

void RelativeView::getPositionAndOrientation(Vector *pos, Matrix3 *orient) {
    Vector up, right, front;
    position->getOrientation(&up, &right, &front);
    Matrix3 M = MatrixFromColumns(right, up, front);
    *pos = position->getLocation() + M*p;
    *orient = M;
}

