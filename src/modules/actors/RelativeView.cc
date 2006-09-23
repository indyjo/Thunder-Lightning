#include <interfaces/IActor.h>
#include "RelativeView.h"

RelativeView::RelativeView(
	Ptr<IActor> subject,
    const Vector & p,
    const Vector & right,
    const Vector & up,
    const Vector & front,
    Ptr<IDrawable> gunsight)
:   subject(subject),
    position(subject),
    movement(subject),
    p(p),
    up(up), right(right), front(front),
    gunsight(gunsight)
{ }

RelativeView::RelativeView(Ptr<IPositionProvider> position,
                           Ptr<IMovementProvider> movement,
                           Ptr<IActor> subject,
                           Ptr<IDrawable> gunsight)
:   subject(subject),
    position(position),
    movement(movement),
    p(0,0,0),
    up(0,1,0),right(1,0,0),front(0,0,1),
    gunsight(gunsight)
{ }

void RelativeView::setViewOffset(Vector p, Vector right, Vector up, Vector front) {
    this->p = p;
    this->up = up;
    this->right = right;
    this->front = front;
}

Vector RelativeView::getLocation() {
    Vector r, u, f;
    position->getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    return position->getLocation() + M*p;
}

Vector RelativeView::getFrontVector() {
    Vector r, u, f;
    position->getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    return M*front;
}

Vector RelativeView::getRightVector() {
    Vector r, u, f;
    position->getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    return M*right;
}
Vector RelativeView::getUpVector() {
    Vector r, u, f;
    position->getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    return M*up;
}
void RelativeView::getOrientation
        (Vector *up, Vector *right, Vector *front) {
    Vector r, u, f;
    position->getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    *up = M*this->up;
    *right = M*this->right;
    *front = M*this->front;
}

Vector RelativeView::getMovementVector() {
	return movement->getMovementVector();
}

Ptr<IActor> RelativeView::getViewSubject() {
    return subject;
}

Ptr<IDrawable> RelativeView::getGunsight() {
    return gunsight;
}
