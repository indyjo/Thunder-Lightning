#include <modules/environment/environment.h>
#include <modules/jogi/JRenderer.h>
#include <modules/scripting/IoScriptingManager.h>
#include <modules/scripting/mappings.h>
#include <interfaces/IConfig.h>
#include <interfaces/IFontMan.h>
#include <interfaces/IFontMetrics.h>
#include <algorithm>

#include "Surface.h"
#include "Console.h"

using namespace std;

namespace {
	string longest_prefix(vector<string> & vec) {
		if (vec.empty()) return "";
		string prefix = vec[0];
		for(int i=1; i<vec.size(); ++i) {
			if (vec[i].size() < prefix.size())
				prefix = prefix.substr(0, vec[i].size());
			for(int j=0; j<prefix.size(); ++j) {
				if (prefix[j] != vec[i][j]) {
					prefix = prefix.substr(0, j);
					break;
				}
			}
		}
		return prefix;
	}
}

namespace UI {

Console::Console(IGame * game)
:   game(game),
    bracecount(0),
    cursor_pos(0),
    enabled(false)
{
    history_pointer = command_history.end();
    game->getEventRemapper()->map("toggle-console",
        SigC::slot(*this, & Console::enable));

    Ptr<IFontMan> fontman = game->getFontMan();
    fontman->selectFont(IFontMan::FontSpec("dungeon", 12));

	IoState* state = game->getIoScriptingManager()->getMainState();
	connectTo(state);
}


void Console::putString(const char * str) {
    for(int i=0; str[i] != 0; i++)
        putChar(str[i]);
}

void Console::putChar(char c) {
	if (lines.empty()) lines.push_back("");
    switch(c) {
    case '\b':
        if (lines.back().empty()) return;
        lines.back() = lines.back().substr(0, lines.back().size()-1);
        break;
    case '\n':
    case '\r':
        lines.push_back("");
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
        //if (lines.back().size() == max_chars)
        //    lines.push_back("");
        lines.back().append(1,c);
    }
}

void Console::draw(JRenderer *renderer) {
    if (!enabled) return;
    Surface surface= Surface::FromCamera(
        renderer->getAspect(),
        renderer->getFocus(),
        renderer->getWidth(),
        renderer->getHeight());
    Ptr<IFontMan> fontman = game->getFontMan();

    renderer->setCoordSystem(JR_CS_EYE);
    renderer->enableAlphaBlending();
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->disableZBuffer();
    renderer->setClipRange(0.1, 10.0);

    renderer->pushMatrix();
    renderer->multMatrix(surface.getMatrix());

    renderer->begin(JR_DRAWMODE_TRIANGLE_FAN);
    renderer->setColor(Vector(0,0,0.5));
    renderer->setAlpha(0.3);
    (*renderer)
        << Vector(0,0,0)
        << Vector(surface.getWidth(),0,0)
        << Vector(surface.getWidth(),surface.getHeight(),0)
        << Vector(0,surface.getHeight(),0);
    renderer->end();

    fontman->selectFont(IFontMan::FontSpec("dungeon", 12));
    fontman->setAlpha(1);
    fontman->setColor(Vector(0,1,0));
    fontman->setCursor(Vector(0,0,0), Vector(1,0,0), Vector(0,1,0));

    size_t max_lines = (size_t) (surface.getHeight() / fontman->getMetrics()->getLineHeight());
    max_lines = std::min(max_lines, lines.size());
    for(int i=lines.size()-max_lines; i<lines.size(); i++) {
        fontman->print(lines[i].c_str());
        if (i!=lines.size()-1) fontman->print("\n");
    }
    fontman->setColor(Vector(.8,.8,.8));
    if (bracecount > 0) {
    	fontman->print("> ");
    } else {
    	fontman->print("# ");
    }
    fontman->print(command.substr(0,cursor_pos).c_str());
    fontman->print("_");
    fontman->print(command.substr(cursor_pos).c_str());

    renderer->popMatrix();

    Ptr<Environment> env = game->getEnvironment();
    renderer->setClipRange(env->getClipMin(), env->getClipMax());
    renderer->enableZBuffer();
    renderer->setCoordSystem(JR_CS_WORLD);
    renderer->disableAlphaBlending();
}

bool Console::feedEvent(SDL_Event & ev) {
    if (ev.type == SDL_KEYDOWN) {
        Uint16 unicode = ev.key.keysym.unicode;
        //ls_message("Console::feedEvent(%d key:%s)\n", unicode, SDL_GetKeyName(ev.key.keysym.sym));

        if (ev.key.keysym.sym == SDLK_F11) {
            disable();
            return false;
        } else if (ev.key.keysym.sym == SDLK_UP) {
        	if (history_pointer == command_history.end()) {
        		history_pointer = command_history.begin();
        	} else {
        		history_pointer++;
        	}
        	if (history_pointer != command_history.end()) {
        		command = *history_pointer;
        	}
        	cursor_pos = std::min(command.size(),cursor_pos);
        } else if (ev.key.keysym.sym == SDLK_DOWN) {
        	if (history_pointer == command_history.begin()) {
        		history_pointer = command_history.end();
        	}
        	if (history_pointer != command_history.begin()) {
        		history_pointer--;
        		command = *history_pointer;
        	}
        	cursor_pos = std::min(command.size(),cursor_pos);
        } else if (ev.key.keysym.sym == SDLK_LEFT
        		   && cursor_pos>0)
        {
        	cursor_pos--;
        } else if (ev.key.keysym.sym == SDLK_RIGHT
        		   && cursor_pos<command.size())
        {
        	cursor_pos++;
        } else if (ev.key.keysym.sym == SDLK_HOME) {
        	cursor_pos=0;
        } else if (ev.key.keysym.sym == SDLK_END) {
        	cursor_pos=command.size();
        } else if (unicode == '\t') {
        	IoState * state = game->getIoScriptingManager()->getMainState();
            IoState_pushRetainPool(state);
        	IoObject * self = IoObject_rawGetSlot_(state->lobby,
        		IoState_symbolWithCString_(state, "complete"));
        	if (self) {
				IoMessage *msg =
                    IoMessage_newWithName_label_(IOSTATE, IOSYMBOL("complete"), IOSYMBOL("Console completion"));
				IoState_retain_(IOSTATE,msg);
				
				char cmd[1024];
				strncpy(cmd,(buffer+command).c_str(), 1024);
				IoMessage_setCachedArg_to_(msg, 0,
					IoSeq_newWithCString_(IOSTATE, cmd));
				IoMessage_setCachedArg_to_(msg, 1, state->lobby);
				
				IoObject *results =IoMessage_locals_performOn_(
					msg,IOSTATE->lobby,IOSTATE->lobby);
				std::vector<std::string> str_results =
					unwrapObject<std::vector<std::string> >(results);
    				
				std::string prefix = longest_prefix(str_results);
				command += prefix;
				cursor_pos += prefix.length();
				if (prefix == "" && !str_results.empty()) {
					for(int i=0; i<str_results.size(); ++i) {
						putString("  ");
						putString(str_results[i].c_str());
						putString("\n");
					}
				}
				IoState_stopRetaining_(IOSTATE, msg);
        	}
            IoState_popRetainPool(state);
        } else if (unicode == '\r') {
        	if (bracecount == 0) putString("# ");
        	else 				 putString("> ");
        	putString(command.c_str());
			putChar('\n');
			
			buffer += '\n';
			buffer += command;
			
			bracecount +=
				std::count(command.begin(),command.end(),'(')
				- std::count(command.begin(),command.end(),')');
			
        	if (bracecount == 0) {
        	    ls_message("Console: executing Io code: %s\n", buffer.c_str());
	        	IoState* state = game->getIoScriptingManager()->getMainState();
				IoObject* result = IoState_doCString_(
					state,
					const_cast<char*>(buffer.c_str()));
					
				if (result) {
					putString(" ==> ");
                    IoObject * printresult = IoState_tryToPerform(state, result, state->lobby, state->printMessage);
                    if (!printresult) {
                        putString("[An exception occurred while printing]\n");
                    } else putChar('\n');
				} else {
					putString("???\n");
				}
				buffer = "";
        	} else if (bracecount < 0) {
        		putString("Error: invalid bracing.\n");
        		bracecount = 0;
        	}
			
			command_history.push_front(command);
        	history_pointer = command_history.end();
			command = "";
			cursor_pos = 0;
        } else if (unicode == '\b') {
        	if (cursor_pos > 0) {
	        	command = command.substr(0,cursor_pos-1) +
	        			  command.substr(cursor_pos);
	        	cursor_pos--;
        	}
        } else if (ev.key.keysym.sym == SDLK_DELETE) {
        	if (cursor_pos < command.size()) {
	        	command = command.substr(0,cursor_pos) +
	        			  command.substr(cursor_pos+1);
        	}
        } else if (unicode > 0 && unicode < 256) {
            command = command.substr(0,cursor_pos)
            		  +(char) unicode
            		  +command.substr(cursor_pos);
            cursor_pos++;
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
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
}

void Console::disable() {
    game->getEventRemapper()->popEventFilter();
    enabled = false;
    SDL_EnableKeyRepeat(0, 0);
}

void Console::printCallback(const char *s) {
	ls_message("%s",s);
	putString(s);
}

void Console::exceptionCallback(IoObject *ex) {
	ls_error("Io Exception\n");
	IoCoroutine_rawPrintBackTrace(ex);
}

void Console::exitCallback() {
	putString("EXIT\n");
    game->getEventRemapper()->triggerAction("endgame");
}


} // namespace UI
