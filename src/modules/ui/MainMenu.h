#ifndef TNL_MAINMENU_H
#define TNL_MAINMENU_H

#include <CEGUI.h>
#include "Dialog.h"

namespace UI {
    class MainGUI;
    
    class MainMenu: public Dialog
    {
        MainGUI & main_gui;
    public:
        inline MainMenu(MainGUI & gui) : main_gui(gui) { }
            
        virtual void enable(MainGUI & main_gui);
        virtual void disable(MainGUI & main_gui);
        bool resume(const CEGUI::EventArgs &);
        bool exitGame(const CEGUI::EventArgs &);
        bool loadMission(const CEGUI::EventArgs &);
        bool editSettings(const CEGUI::EventArgs &);
    };
} // namespace UI

#endif


