#include <interfaces/IActor.h>
#include <modules/gunsight/gunsight.h>

#include "SimpleView.h"

SimpleView::SimpleView(Ptr<IActor> subject, Ptr<IDrawable> gunsight)
:	subject(subject),
	gunsight(gunsight)
{ }

Vector SimpleView::getLocation() {
	Vector pos;
	Matrix3 orient;
	getPositionAndOrientation(&pos, &orient);
	return pos;
}

Vector SimpleView::getFrontVector() {
	Vector pos;
	Matrix3 orient;
	getPositionAndOrientation(&pos, &orient);
	return orient*Vector(0,0,1);
}

Vector SimpleView::getRightVector() {
	Vector pos;
	Matrix3 orient;
	getPositionAndOrientation(&pos, &orient);
	return orient*Vector(1,0,0);
}

Vector SimpleView::getUpVector() {
	Vector pos;
	Matrix3 orient;
	getPositionAndOrientation(&pos, &orient);
	return orient*Vector(0,1,0);
}

void SimpleView::getOrientation(Vector * up, Vector * right, Vector * front)
{
	Vector pos;
	Matrix3 orient;
	getPositionAndOrientation(&pos, &orient);
	*up = orient*Vector(0,1,0);
	*right = orient*Vector(1,0,0);
	*front = orient*Vector(0,0,1);
}

Vector SimpleView::getMovementVector() {
	return subject->getMovementVector();
}

Ptr<IActor> SimpleView::getViewSubject() {
	return subject;
}

Ptr<IDrawable> SimpleView::getGunsight() {
	return gunsight;
}

void SimpleView::enable() {
    on_enable.emit();
    if(gunsight) gunsight->enable();
}

void SimpleView::disable() {
    on_disable.emit();
    if(gunsight) gunsight->disable();
}

SigC::Signal0<void> & SimpleView::onEnable() {
    return on_enable;
}

SigC::Signal0<void> & SimpleView::onDisable() {
    return on_disable;
}

