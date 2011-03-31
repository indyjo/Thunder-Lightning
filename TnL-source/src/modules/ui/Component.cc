#include "Component.h"

namespace UI {

Component::Component(
	const char *name, float w, float h)
:   width(w), height(h), name(name), offset(0,0)
{ }

void Component::draw(Panel &) { }

float Component::getWidth() { return width; }
float Component::getHeight() { return height; }

void Component::enable() { }
void Component::disable() { }

void Component::onLayout(Panel &) { }

} // namespace UI

