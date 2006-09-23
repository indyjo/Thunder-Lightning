#include "Idea.h"

namespace AI {

Idea::Idea(const std::string & name) : name(name), enabled(true) { }

void Idea::setEnabled(bool e) {
    if (e && !enabled) {
        enabled = e;
        onEnabled();
    } else if ( !e && enabled ) {
        enabled = e;
        onDisabled();
    }
}

void Idea::think() {
    if (enabled) onThink();
}

void Idea::onEnabled() { }
void Idea::onDisabled() { }
void Idea::onThink() { }

} // namespace AI
