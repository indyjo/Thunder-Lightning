#ifdef HAVE_CEGUI

#include "Dialog.h"
#include "MainGUI.h"
#include "MainMenu.h"

#include <CEGUIBase/CEGUI.h>

namespace UI {
    void MainMenu::enable(MainGUI & main_gui) {
        CEGUI::Window *window = CEGUI::WindowManager::getSingleton().loadWindowLayout(
            "mainDialog.layout");
        CEGUI::System::getSingleton().setGUISheet(window);
        
        // resume game
        window->getChild("mainDialog")->
        getChild("mainDialog/content")->
        getChild("mainDialog/content/resumeGame")->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&MainMenu::resume, this));

        // exit game
        window->getChild("mainDialog")->
        getChild("mainDialog/content")->
        getChild("mainDialog/content/exitGame")->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&MainMenu::exitGame, this));

        // load mission
        window->getChild("mainDialog")->
        getChild("mainDialog/content")->
        getChild("mainDialog/content/loadMission")->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&MainMenu::loadMission, this));

        // edit settings
        window->getChild("mainDialog")->
        getChild("mainDialog/content")->
        getChild("mainDialog/content/settings")->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&MainMenu::editSettings, this));
    }
    void MainMenu::disable(MainGUI & main_gui) {
        CEGUI::Window *window = CEGUI::System::getSingleton().getGUISheet();
        CEGUI::WindowManager::getSingleton().destroyWindow(window);
    }
    bool MainMenu::resume(const CEGUI::EventArgs &) {
        main_gui.switchOff();
        return true;
    }
    bool MainMenu::exitGame(const CEGUI::EventArgs &) {
        Ptr<IGame> game = main_gui.getGame();
        if (game) game->endGame();
        return true;
    }
    bool MainMenu::loadMission(const CEGUI::EventArgs &) {
        main_gui.switchToMissionSelector();
        return true;
    }
    bool MainMenu::editSettings(const CEGUI::EventArgs &) {
        main_gui.switchToSettingsDlg();
        return true;
    }

} // namespace UI

#endif // HAVE_CEGUI
