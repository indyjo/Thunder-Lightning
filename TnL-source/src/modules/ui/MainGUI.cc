#include <CEGUIBase/CEGUI.h>
#include <CEGUIEventFilter.h>
#include <remap.h>
#include "Dialog.h"
#include "MainMenu.h"
#include "MissionSelector.h"
#include "SettingsDlg.h"
#include "MainGUI.h"

namespace UI {
    MainGUI::MainGUI(Ptr<IGame> game)
        : game(game)
        , ui_state(OFF)
    { }
    
    MainGUI::~MainGUI() {
        if (current_dlg) {
            current_dlg->disable(*this);
            current_dlg = 0;
        }
    }
    
    MainGUI::UIState MainGUI::currentState() {
        return ui_state;
    }
    
    void MainGUI::switchOff() {
        switchToState(OFF, 0);
    }
    
    void MainGUI::switchToMainMenu(bool allow_resume, bool allow_save) {
        switchToState(MAIN_MENU, new MainMenu(*this));
    }
    
    void MainGUI::switchToState(UIState new_state, Ptr<Dialog> new_dlg) {
        Ptr<IGame> thegame = game.lock();
        if (!thegame) return;
        
        if (new_state == ui_state) return;
        
        if (ui_state != OFF) {
            thegame->getEventRemapper()->popEventFilter();
        }
        if (current_dlg) {
            current_dlg->disable(*this);
        }

        ui_state = new_state;
        
        current_dlg = new_dlg;
        if (current_dlg) {
            current_dlg->enable(*this);
        }
        
        CEGUI::MouseCursor::getSingleton().setVisible(ui_state != OFF);
        
        if (ui_state != OFF) {
            thegame->getEventRemapper()->pushEventFilter(new CEGUIEventFilter(*thegame, false));
        }
    }
    
    void MainGUI::switchToMissionSelector() {
        switchToState(MISSION_SELECTOR, new MissionSelector(*this));
    }
    
    void MainGUI::switchToSettingsDlg() {
        Ptr<IGame> thegame = game.lock();
        if (!thegame) return;
        switchToState(SETTINGS_DLG, new SettingsDlg(*this, thegame->getConfig(), thegame->getEventRemapper()));
    }

} // namespace UI

