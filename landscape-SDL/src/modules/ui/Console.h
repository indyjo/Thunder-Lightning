#ifndef UI_CONSOLE_H
#define UI_CONSOLE_H

#include <deque>
#include <string>
#include <landscape.h>
#include <remap.h>

namespace UI {

class Surface;

class ICommandHandler : virtual public Object {
public:
    virtual std::string handle(const std::string & cmd) = 0;
};

class Console : public IEventFilter, virtual public SigObject
{
    typedef std::deque<std::string> Lines;

    IGame * game;
    Lines lines;
    Matrix coord_sys;

    int max_chars, max_lines;
    bool enabled;

    std::vector<Ptr<ICommandHandler> > cmd_handlers;

public:
    Console(IGame * game,
            const Surface & surface,
            int max_chars = 80,
            int max_lines = 20);

    void putString(const char * str);
    void putChar(char c);

    void draw(JRenderer *renderer);

    void enable();
    void disable();

    void pushCommandHandler(Ptr<ICommandHandler> handler);
    void popCommandHandler();

    virtual bool feedEvent(SDL_Event & ev);
}; // class Console

} // namespace UI

#endif
