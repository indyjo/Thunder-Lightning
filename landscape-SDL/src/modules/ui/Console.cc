#include <modules/environment/environment.h>
#include <modules/jogi/JRenderer.h>
#include <interfaces/IConfig.h>
#include <interfaces/IFontMan.h>


#include "Surface.h"
#include "Console.h"

using namespace std;

namespace UI {

class DefaultCommandHandler: public ICommandHandler
{
    IGame * game;
protected:
    string usage() {
        return "Usage: [get] var_name\n";
    }
    string cmdGet(vector<string> & params) {
        if (params.size() != 2)
            return usage();
        return string(
            game->getConfig()->query(params[1].c_str(), "<not found>"))
            +"\n";
    }
    string handle(vector<string> & params) {
        if (params.empty()) {
            return usage();
        }
        if (params[0] == "get") {
            return cmdGet(params);
        } else {
            return usage();
        }
    }
public:
    DefaultCommandHandler(IGame * game) : game(game) { }

    virtual string handle(const string & cmd) {
        vector<string> params;
        int n1=0, n2=0;
        while (n2 != string::npos) {
            n1 = cmd.find_first_not_of(" \t", n2);
            if (n1 == string::npos) break;
            n2 = cmd.find_first_of(" \t", n1);
            if (n2 == string::npos) {
                params.push_back(cmd.substr(n1, n2));
                break;
            }
            params.push_back(cmd.substr(n1, n2-n1));
        }
        return handle(params);
    }


};

Console::Console(IGame * game,
                 const Surface & surface,
                 int max_chars,
                 int max_lines)
:   game(game),
    coord_sys(surface.getMatrix()),
    max_chars(max_chars),
    max_lines(max_lines),
    enabled(false)
{
    lines.push_back("");
    game->getEventRemapper()->map("toggle-console",
        SigC::slot(*this, & Console::enable));
    pushCommandHandler(new DefaultCommandHandler(game));
}


void Console::putString(const char * str) {
    for(int i=0; str[i] != 0; i++)
        putChar(str[i]);
}

void Console::putChar(char c) {
    switch(c) {
    case '\b':
        if (lines.back().empty()) return;
        lines.back() = lines.back().substr(0, lines.back().size()-1);
        break;
    case '\n':
    case '\r':
        lines.push_back("");
        if (lines.size() > max_lines)
            lines.pop_front();
        break;
    case '\t':
        {
            int n=lines.back().size();
            n = (n+7) & -8;
            n -= lines.back().size();
            lines.back().append(n,' ');
        }
        break;
    default:
        if (lines.back().size() == max_chars)
            return;
        lines.back().append(1,c);
    }
}

void Console::draw(JRenderer *renderer) {
    if (!enabled) return;
    Ptr<IFontMan> fontman = game->getFontMan();

    renderer->setCoordSystem(JR_CS_EYE);
    renderer->enableAlphaBlending();
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->disableZBuffer();
    renderer->setClipRange(0.1, 10.0);

    renderer->pushMatrix();
    renderer->multMatrix(coord_sys);

    renderer->begin(JR_DRAWMODE_TRIANGLE_FAN);
    renderer->setColor(Vector(0,0,0.5));
    renderer->setAlpha(0.3);
    (*renderer) << Vector(0,0,0) << Vector(1024,0,0)
                << Vector(1024,768,0) << Vector (0,768,0);
    renderer->end();

    fontman->selectFont(IFontMan::FontSpec("dungeon", 12));
    fontman->setAlpha(1);
    fontman->setColor(Vector(0,0,0));
    fontman->setCursor(Vector(1,1,0), Vector(1,0,0), Vector(0,1,0));

    for(int i=0; i<lines.size(); i++) {
        fontman->print(lines[i].c_str());
        if (i==lines.size()-1) {
            fontman->print("_"); // draw a cursor
        } else {
            fontman->print("\n");
        }
    }
    fontman->setColor(Vector(0,1,0));
    fontman->setCursor(Vector(0,0,0), Vector(1,0,0), Vector(0,1,0));

    for(int i=0; i<lines.size(); i++) {
        fontman->print(lines[i].c_str());
        if (i==lines.size()-1) {
            fontman->print("_"); // draw a cursor
        } else {
            fontman->print("\n");
        }
    }

    renderer->popMatrix();

    Ptr<Environment> env = game->getEnvironment();
    renderer->setClipRange(env->getClipMin(), env->getClipMax());
    renderer->enableZBuffer();
    renderer->setCoordSystem(JR_CS_WORLD);
    renderer->disableAlphaBlending();
}

bool Console::feedEvent(SDL_Event & ev) {
    if (ev.type == SDL_KEYDOWN) {
        if (ev.key.keysym.sym == SDLK_CARET) {
            disable();
            return false;
        }
        Uint16 unicode = ev.key.keysym.unicode;
        ls_message("Console::feedEvent(%d)\n", unicode);

        if (unicode >= 32 && unicode < 256
            || unicode == '\n'
            || unicode == '\r'
            || unicode == '\t'
            || unicode == '\b')
        {
            if (unicode == '\r') {
                string cmd = lines.back();
                putChar('\n');
                putString(cmd_handlers.back()->handle(cmd).c_str());
            } else {
                putChar((char) unicode);
            }
        }
        return false;
    } else if (ev.type == SDL_KEYUP) {
        return false;
    }
    return true;
}

void Console::enable() {
    game->getEventRemapper()->pushEventFilter(this);
    enabled = true;
}

void Console::disable() {
    game->getEventRemapper()->popEventFilter();
    enabled = false;
}

void Console::pushCommandHandler(Ptr<ICommandHandler> handler) {
    cmd_handlers.push_back(handler);
}
void Console::popCommandHandler() {
    cmd_handlers.pop_back();
}

} // namespace UI
