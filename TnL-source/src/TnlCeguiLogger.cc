#include <debug.h>
#include "TnlCeguiLogger.h"

void TnlCeguiLogger::logEvent(const CEGUI::String& message, CEGUI::LoggingLevel level) {
    
    using namespace CEGUI;
    
    switch(level) {
        case Standard:
        case Informative:
#ifndef NDEBUG
        case Insane:
#endif
            ls_message("CEGUI: %s\n", message.c_str());
            break;
        case Errors:
            ls_error("CEGUI Error: %s\n", message.c_str());
            break;
    }
}


