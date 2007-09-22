#ifndef TNL_MISSIONSELECTOR_H
#define TNL_MISSIONSELECTOR_H

#include <CEGUI.h>
#include "Dialog.h"

namespace UI {
    class MainGUI;
    class MainMenu;
    
    class MissionSelector : public Dialog
    {
    public:
        MissionSelector(MainGUI& main_gui);
        
        virtual void enable(MainGUI & main_gui);
        virtual void disable(MainGUI & main_gui);
    private:
        bool back(const CEGUI::EventArgs &);
        bool accept(const CEGUI::EventArgs &);
        bool selectionChanged(const CEGUI::EventArgs &);
        
    private:
        CEGUI::Listbox* list;
        CEGUI::MultiLineEditbox* descbox;
        CEGUI::PushButton* acceptbtn;
        MainGUI & main_gui;
    };
    

} // namespace

#endif

