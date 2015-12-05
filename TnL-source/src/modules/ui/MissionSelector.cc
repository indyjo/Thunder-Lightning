#include <cstdio>
#include <stdexcept>
#include <string>
#include <modules/scripting/mappings.h>
#include <modules/scripting/IoScriptingManager.h>
#include <debug.h>
#include <CEGUIBase/CEGUI.h>
#include "MainGUI.h"
#include "MissionSelector.h"

namespace UI {
    MissionSelector::MissionSelector(MainGUI& main_gui)
        : main_gui(main_gui)
    { }
    
    void MissionSelector::enable(MainGUI & main_gui) {
        Ptr<IGame> game = main_gui.getGame();
        if (!game) {
            ls_error("MissionSelector::enable() called after lifetime of Game has ended.\n");
            throw std::runtime_error("MissionSelector::enable() called after lifetime of Game has ended.");
        }
        
        CEGUI::Window *window = CEGUI::WindowManager::getSingleton().loadWindowLayout(
            "missionDialog.layout");
        CEGUI::System::getSingleton().setGUISheet(window);
        
        // back button handler (don't accept mission, return to parent)
        window->getChild("missionDialog")->
        getChild("missionDialog/content")->
        getChild("missionDialog/content/back")->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&MissionSelector::back, this));
            
        list = (CEGUI::Listbox*)
        window->getChild("missionDialog")->
        getChild("missionDialog/content")->
        getChild("missionDialog/content/missionList");
        
        descbox = (CEGUI::MultiLineEditbox*)
        window->getChild("missionDialog")->
        getChild("missionDialog/content")->
        getChild("missionDialog/content/missionDescription");
        
        acceptbtn = (CEGUI::PushButton*)
        window->getChild("missionDialog")->
        getChild("missionDialog/content")->
        getChild("missionDialog/content/acceptMission");
        acceptbtn->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&MissionSelector::accept, this));
        
        IoState * state = game->getIoScriptingManager()->getMainState();
        IoObject* result = IoState_doCString_(state, "MissionManager numMissions");
        if (!ISNUMBER(result)) {
            ls_error("Couldn't retrieve number of missions from MissionManager\n");
            ls_error("(MissionManager numMissions) must return a number.\n");
            throw std::runtime_error("Couldn't receive number of missions from MissionManager");
        }
        
        int num_missions = unwrapObject<int>(result);
        for (int i=0; i<num_missions; ++i) {
            char cmd[80];
            snprintf(cmd, 80, "MissionManager missionName(%d)", i);
            result = IoState_doCString_(state, cmd);
            if (!ISSEQ(result)) {
                ls_error("Couldn't retrieve mission name from MissionManager\n");
                ls_error("MissionManager missionName(<int>) must return a sequence.\n");
                throw std::runtime_error("Couldn't receive name of mission from MissionManager");
            }
            
            list->addItem(new CEGUI::ListboxTextItem(CSTRING(result),i));
        }
        list->subscribeEvent(
            CEGUI::Listbox::EventSelectionChanged,
            CEGUI::SubscriberSlot(&MissionSelector::selectionChanged, this));
        list->setItemSelectState((size_t)0, true);
        
    }
    
    void MissionSelector::disable(MainGUI & main_gui) {
        CEGUI::Window *window = CEGUI::System::getSingleton().getGUISheet();
        CEGUI::WindowManager::getSingleton().destroyWindow(window);
    }
    
    bool MissionSelector::back(const CEGUI::EventArgs &) {
        main_gui.switchToMainMenu(false, false);
        return true;
    }
    
    bool MissionSelector::accept(const CEGUI::EventArgs &) {
        Ptr<IGame> game = main_gui.getGame();
        if (!game) {
            throw std::runtime_error("MissionSelector::accept() called after lifetime of Game has ended.");
        }
        
        game->restartSimulation();
        
        CEGUI::ListboxItem *item = list->getFirstSelectedItem();
        if (item) {
            IoState * state = game->getIoScriptingManager()->getMainState();
            char cmd[80];
            snprintf(cmd, 80, "MissionManager runMission(%d)", item->getID());
            IoObject * result = IoState_doCString_(state, cmd);
        }
        main_gui.switchOff();
        return true;
    }
    
    bool MissionSelector::selectionChanged(const CEGUI::EventArgs &) {
        Ptr<IGame> game = main_gui.getGame();
        if (!game) {
            ls_error("MissionSelector::selectionChanged() called after lifetime of Game has ended.\n");
            throw std::runtime_error("MissionSelector::selectionChanged() called after lifetime of Game has ended.");
        }

        CEGUI::ListboxItem *item = list->getFirstSelectedItem();
        if (item) {
            IoState * state = game->getIoScriptingManager()->getMainState();
            char cmd[80];
            snprintf(cmd, 80, "MissionManager missionDesc(%d)", item->getID());
            IoObject * result = IoState_doCString_(state, cmd);
            if (!ISSEQ(result)) {
                ls_error("Couldn't retrieve mission description from MissionManager\n");
                ls_error("MissionManager missionDesc(<int>) must return a sequence.\n");
                throw std::runtime_error("Couldn't receive description of mission from MissionManager");
            }
            acceptbtn->setVisible(true);
            descbox->setText(CSTRING(result));
        } else {
            acceptbtn->setVisible(false);
            descbox->setText("");
        }
        return true;
    }
} // namespace UI

