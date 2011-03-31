#include <RenderPass.h>
#include <interfaces/IActor.h>

#include "SimpleView.h"

SimpleView::SimpleView(Ptr<IActor> subject, Ptr<IMovementProvider> head, Ptr<RenderPass> pass)
:	subject(subject),
    head(head),
    render_pass(pass),
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

Ptr<IActor> SimpleView::getViewSubject() {
	return subject;
}

Ptr<IMovementProvider> SimpleView::getViewHead() {
    return head;
}

Ptr<RenderPass> SimpleView::getRenderPass() {
    return render_pass;
}


void SimpleView::enable() {
    if (is_enabled) return;
    is_enabled = true;
    on_enable.emit();
    if(event_sheet) event_remapper->addEventSheet(event_sheet);
}

void SimpleView::disable() {
    if (!is_enabled) return;
    is_enabled = false;
    on_disable.emit();
    if(event_sheet) event_remapper->removeEventSheet(event_sheet);
}

bool SimpleView::isEnabled() {
    return is_enabled;
}

SigC::Signal0<void> & SimpleView::onEnable() {
    return on_enable;
}

SigC::Signal0<void> & SimpleView::onDisable() {
    return on_disable;
}


