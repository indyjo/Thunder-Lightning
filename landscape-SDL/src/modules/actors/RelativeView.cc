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
    p(p),
    up(up), right(right), front(front),
    gunsight(gunsight)
{ }

Vector RelativeView::getLocation() {
    Vector r, u, f;
    subject->getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    return subject->getLocation() + M*p;
}

Vector RelativeView::getFrontVector() {
    Vector r, u, f;
    subject->getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    return M*front;
}

Vector RelativeView::getRightVector() {
    Vector r, u, f;
    subject->getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    return M*right;
}
Vector RelativeView::getUpVector() {
    Vector r, u, f;
    subject->getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    return M*up;
}
void RelativeView::getOrientation
        (Vector *up, Vector *right, Vector *front) {
    Vector r, u, f;
    subject->getOrientation(&u, &r, &f);
    Matrix3 M = MatrixFromColumns<float>(r,u,f);
    *up = M*this->up;
    *right = M*this->right;
    *front = M*this->front;
}

Vector RelativeView::getMovementVector() {
	return subject->getMovementVector();
}

Ptr<IActor> RelativeView::getViewSubject() {
    return subject;
}

Ptr<IDrawable> RelativeView::getGunsight() {
    return gunsight;
}
