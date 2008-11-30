#ifndef TNL_CEGUILOGGER_H
#define TNL_CEGUILOGGER_H

#include <CEGUILogger.h>

class TnlCeguiLogger : public CEGUI::Logger {
    virtual void logEvent(const CEGUI::String& message, CEGUI::LoggingLevel level);
    virtual void setLogFilename(const CEGUI::String&, bool) {};
};

#endif
