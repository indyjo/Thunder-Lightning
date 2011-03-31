#ifndef UI_CONSOLE_H
#define UI_CONSOLE_H

#include <deque>
#include <string>
#include <tnl.h>
#include <remap.h>
#include <interfaces/IGame.h>
#include <modules/scripting/IoScriptingManager.h>

class JRenderer;

namespace UI {
class Surface;

class Console : public IEventFilter, public IoCallbackContext, virtual public SigObject
{
    typedef std::deque<std::string> Lines;
    typedef Lines::iterator LinesIter;

    IGame * game;

    Lines lines, command_history;
    LinesIter history_pointer;
    std::string command, buffer;
    int bracecount;
    size_t cursor_pos;

    bool enabled;

public:
    Console(IGame * game);

    void putString(const char * str);
    void putChar(char c);

    void draw(JRenderer *renderer);

    void enable();
    void disable();

    virtual bool feedEvent(SDL_Event & ev);
    
	virtual void printCallback(const char *);
	virtual void exceptionCallback(IoObject *);
	virtual void exitCallback();
}; // class Console

} // namespace UI

#endif
