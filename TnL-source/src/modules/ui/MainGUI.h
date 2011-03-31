#ifndef TNL_MAINGUI_H
#define TNL_MAINGUI_H

#include <CEGUI.h>
#include <interfaces/IGame.h>
#include <Weak.h>

namespace UI {
    class Dialog;
    
    class MainGUI : public SigObject {
    public:
        MainGUI(Ptr<IGame> game);
        virtual ~MainGUI();
    
        enum UIState {
            OFF,
            MAIN_MENU,
            MISSION_SELECTOR,
            SETTINGS_DLG
        };
        
        UIState currentState();
        
        void switchOff();
        void switchToMainMenu(bool allow_resume, bool allow_save);
        void switchToMissionSelector();
        void switchToSettingsDlg();
        
        inline Ptr<IGame> getGame() { return game.lock(); }
        
    private:
        void switchToState(UIState new_state, Ptr<Dialog> new_dlg);
    
        WeakPtr<IGame> game;
        UIState ui_state;
        Ptr<Dialog> current_dlg;
    };
} // namespace UI

#endif

