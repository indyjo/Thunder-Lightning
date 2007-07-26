#include <interfaces/IActor.h>
#include <modules/gunsight/gunsight.h>

#include "SimpleView.h"

SimpleView::SimpleView(Ptr<IActor> subject, Ptr<IDrawable> gunsight)
:	subject(subject),
	gunsight(gunsight),
	is_enabled(false)
{ }

Ptr<EventSheet> SimpleView::getEventSheet(Ptr<EventRemapper> remap) {
    if (!event_sheet) {
        event_sheet = new EventSheet;
        event_remapper = remap;
        if (isEnabled()) {
            event_remapper->addEventSheet(event_sheet);
        }
    }
    return event_sheet;
}

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
    if (is_enabled) return;
    is_enabled = true;
    on_enable.emit();
    if(event_sheet) event_remapper->addEventSheet(event_sheet);
    if(gunsight) gunsight->enable();
}

void SimpleView::disable() {
    if (!is_enabled) return;
    is_enabled = false;
    on_disable.emit();
    if(gunsight) gunsight->disable();
    if(event_sheet) event_remapper->removeEventSheet(event_sheet);
}

SigC::Signal0<void> & SimpleView::onEnable() {
    return on_enable;
}

SigC::Signal0<void> & SimpleView::onDisable() {
    return on_disable;
}

bool SimpleView::isEnabled() {
    return is_enabled;
}

Ptr<RenderPass> SimpleView::getRenderPass() { return render_pass; }
void SimpleView::setRenderPass(Ptr<RenderPass> p) {render_pass = p; }

