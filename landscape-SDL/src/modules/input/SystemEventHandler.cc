#include "SystemEventHandler.h"

void SystemEventHandler::handle(SDL_Event & ev) {
    if (ev.type == SDL_QUIT) {
        endgame.emit();
        return;
    }
    
    if (ev.type == SDL_KEYDOWN) {
        switch(ev.key.keysym.sym) {
        case: 

    EventHandler::handle(ev);
}

