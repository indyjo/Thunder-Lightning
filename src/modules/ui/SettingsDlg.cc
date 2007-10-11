#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>
#include <SDL.h>
#include <interfaces/IGame.h>
#include <modules/clock/clock.h>
#include <modules/scripting/mappings.h>
#include <modules/scripting/IoScriptingManager.h>
#include <debug.h>
#include <remap.h>

#include "MainGUI.h"
#include "SettingsDlg.h"

namespace {
    class ButtonDetector : public IEventFilter, public SigC::Object
    {
        float time_left;
        EventRemapper::ButtonType type;
    public:
        ButtonDetector(EventRemapper::ButtonType t)
            : time_left(8)
            , type(t)
        { }
        
        virtual bool feedEvent(SDL_Event & ev) {
            if (time_left <= 0) {
                return false;
            }
            
            switch (ev.type) {
            case SDL_KEYDOWN:
                detect(EventRemapper::Button(
                    EventRemapper::KEYBOARD_KEY, 0, ev.key.keysym.sym));
                break;
            case SDL_MOUSEBUTTONDOWN:
                detect(EventRemapper::Button(
                    EventRemapper::MOUSE_BUTTON, 0, ev.button.button));
                break;
            case SDL_JOYBUTTONDOWN:
                detect(EventRemapper::Button(
                    EventRemapper::JOYSTICK_BUTTON, ev.jbutton.which, ev.jbutton.button));
                break;
            }
            
            return false;
        }
        
        void tick(float delta_t) {
            if (time_left > 0) {
                time_left -= delta_t;
                if (time_left <= 0) {
                    cancelled.emit();
                }
            }
        }
        
        SigC::Signal1<void, EventRemapper::Button> button_detected;
        SigC::Signal0<void> cancelled;
        
    private:
        void detect(EventRemapper::Button btn) {
            if ( btn.type == type ) {
                button_detected.emit(btn);
            }
        }
    };
} // namespace

namespace UI {
    SettingsDlg::SettingsDlg(MainGUI& main_gui, Ptr<IConfig> c, Ptr<EventRemapper> r)
        : main_gui(main_gui)
        , cfg(c)
        , remapper(r)
    {
        Ptr<IGame> thegame = main_gui.getGame();
        if (thegame) {
            thegame->post_draw.connect(SigC::slot(
                *this, &SettingsDlg::onFrame));
        }
    }
    
    void SettingsDlg::enable(MainGUI & main_gui) {
        root = CEGUI::WindowManager::getSingleton().loadWindowLayout(
            "settings.layout");
        
        settings_wnd = root->getChild("settingsWnd");
        joystick_enabled_checkbox = (CEGUI::Checkbox*)
            settings_wnd->getChild("settingsWnd/joystickCheckbox");
        fullscreen_auto_radio = (CEGUI::RadioButton*)
            settings_wnd->getChild("settingsWnd/fullscreenAutoRadio");
        fullscreen_fixed_radio = (CEGUI::RadioButton*)
            settings_wnd->getChild("settingsWnd/fullscreenManualRadio");
        windowed_radio = (CEGUI::RadioButton*)
            settings_wnd->getChild("settingsWnd/windowedRadio");
        resolutions_combo = (CEGUI::Combobox*)
            settings_wnd->getChild("settingsWnd/resolutions");
        resolutions_combo->setValidationString("[0-9]+x[0-9]+");
        shaders_enabled_checkbox = (CEGUI::Checkbox*)
            settings_wnd->getChild("settingsWnd/shaderCheckbox");
            
        SDL_Rect **modes = SDL_ListModes(NULL, SDL_OPENGL | SDL_FULLSCREEN);
        while (*modes) {
            char buf[256];
            snprintf(buf, 256, "%dx%d",(int)(*modes)->w, (int)(*modes)->h);
            resolutions_combo->addItem(new CEGUI::ListboxTextItem(buf));
            modes++;
        }
        
        // back button handler
        settings_wnd->getChild("settingsWnd/backBtn")->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::back, this));
        
        // configure buttons and keys handler
        settings_wnd->getChild("settingsWnd/customizeEventsBtn")->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::customizeButtons, this));


        buttons_wnd = root->getChild("eventsWnd");
        events_list = (CEGUI::MultiColumnList *)
            buttons_wnd->getChild("eventsWnd/eventsList");
        events_list->setShowHorzScrollbar(false);
        events_list->setSelectionMode(CEGUI::MultiColumnList::RowSingle);
        events_list->subscribeEvent(
            CEGUI::MultiColumnList::EventSelectionChanged,
            CEGUI::SubscriberSlot(&SettingsDlg::eventSelected, this));

        keyboard_key_button = (CEGUI::PushButton *)
            buttons_wnd->getChild("eventsWnd/keyboardButton");
        keyboard_clear_button = (CEGUI::PushButton *)
            buttons_wnd->getChild("eventsWnd/clearKeyboard");
        joystick_button_button = (CEGUI::PushButton *)
            buttons_wnd->getChild("eventsWnd/joystickButton");
        joystick_clear_button = (CEGUI::PushButton *)
            buttons_wnd->getChild("eventsWnd/clearJoystick");
        mouse_button_button = (CEGUI::PushButton *)
            buttons_wnd->getChild("eventsWnd/mouseButton");
        mouse_clear_button = (CEGUI::PushButton *)
            buttons_wnd->getChild("eventsWnd/clearMouse");
            
        // react on "clear" buttons
        keyboard_clear_button->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::clearButton, this));
        mouse_clear_button->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::clearButton, this));
        joystick_clear_button->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::clearButton, this));

        // react on "configure" buttons
        keyboard_key_button->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::configureButton, this));
        mouse_button_button->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::configureButton, this));
        joystick_button_button->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::configureButton, this));
            
        // react on "OK" button
        buttons_wnd->getChild("eventsWnd/okButton")->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::buttonsCustomized, this));

        wait_for_keyboard_key_wnd = root->getChild("waitForKeyboard");
        wait_for_mouse_button_wnd = root->getChild("waitForMouse");
        wait_for_joystick_button_wnd = root->getChild("waitForJoystick");
        
        CEGUI::System::getSingleton().setGUISheet(root);

        loadFromConfig();
    }
    
    void SettingsDlg::disable(MainGUI & main_gui) {
        CEGUI::WindowManager::getSingleton().destroyWindow(root);
    }
    
    bool SettingsDlg::back(const CEGUI::EventArgs &) {
        saveToConfig();

        Ptr<IGame> game = main_gui.getGame();
        if (!game) {
            throw std::runtime_error("SettingsDlg::back() called after lifetime of Game has ended.");
        }
        
        IoState * state = game->getIoScriptingManager()->getMainState();
        IoState_doCString_(state, "Settings saveConfig");
        IoState_doCString_(state, "Settings saveControls");

        main_gui.switchToMainMenu(false, false);
        return true;
    }
    
    bool SettingsDlg::customizeButtons(const CEGUI::EventArgs &) {
        buttons_wnd->setVisible(true);
        buttons_wnd->setModalState(true);
        loadEvents();
        // Select the first item just to be sure
        if (events_list->getRowCount() > 0) {
            events_list->setItemSelectState(
                events_list->getItemAtGridReference(CEGUI::MCLGridRef(0,0)),
                true);
        }
        return true;
    }

    bool SettingsDlg::buttonsCustomized(const CEGUI::EventArgs &) {
        buttons_wnd->setVisible(false);
        buttons_wnd->setModalState(false);
        return true;
    }


    bool SettingsDlg::eventSelected(const CEGUI::EventArgs &) {
        loadButtons();
        return true;
    }

    bool SettingsDlg::clearButton(const CEGUI::EventArgs & args) {
        // If no event selected, we're done
        if (0 == events_list->getSelectedCount()) {
            return true;
        }
        
        // Get name of selected event
        CEGUI::ListboxItem * item = events_list->getFirstSelectedItem();
        const char *event = item->getText().c_str();

        // Determine from event args which button type to clear
        CEGUI::WindowEventArgs & wargs = (CEGUI::WindowEventArgs&) args;
        EventRemapper::ButtonType type_to_clear = EventRemapper::KEYBOARD_KEY;
        if (wargs.window == mouse_clear_button) {
            type_to_clear = EventRemapper::MOUSE_BUTTON;
        } else if (wargs.window == joystick_clear_button) {
            type_to_clear = EventRemapper::JOYSTICK_BUTTON;
        }
        
        // Unmap all buttons of that type from the event
        remapper->unmapButtonsOfType(type_to_clear, event);
        
        // Update
        loadButtons();
        
        return true;
    }

    bool SettingsDlg::configureButton(const CEGUI::EventArgs & args) {
        // Determine from event args which button type to detect
        CEGUI::WindowEventArgs & wargs = (CEGUI::WindowEventArgs&) args;
        Ptr<ButtonDetector> detector;
        if (wargs.window == keyboard_key_button) {
            wait_for_keyboard_key_wnd->setVisible(true);
            wait_for_keyboard_key_wnd->activate();
            detector = new ButtonDetector(EventRemapper::KEYBOARD_KEY);
        } else if (wargs.window == mouse_button_button) {
            wait_for_mouse_button_wnd->setVisible(true);
            wait_for_mouse_button_wnd->activate();
            detector = new ButtonDetector(EventRemapper::MOUSE_BUTTON);
        } else if (wargs.window == joystick_button_button) {
            wait_for_joystick_button_wnd->setVisible(true);
            wait_for_joystick_button_wnd->activate();
            detector = new ButtonDetector(EventRemapper::JOYSTICK_BUTTON);
        }
        
        if (!detector) {
            return true;
        }
        
        tick.connect(SigC::slot(
            *detector, &ButtonDetector::tick));
        detector->button_detected.connect( SigC::slot(
            *this, &SettingsDlg::buttonDetected));
        detector->cancelled.connect( SigC::slot(
            *this, &SettingsDlg::buttonDetectionCancelled));
        remapper->pushEventFilter(detector);
        
        return true;
    }
    
    void SettingsDlg::onFrame() {
        float delta_t = 0;
        Ptr<IGame> thegame = main_gui.getGame();
        if (thegame) {
            Ptr<Clock> clock = thegame->getClock();
            delta_t = clock->getRealFrameDelta();
        }
        
        tick.emit(delta_t);
    }

    void SettingsDlg::loadFromConfig() {
        joystick_enabled_checkbox->setSelected(
            cfg->queryBool("Controls_enable_joystick"));
        if (cfg->queryBool("Game_fullscreen")) {
            if (cfg->queryBool("Game_auto_resolution")) {
                fullscreen_auto_radio->setSelected(true);
            } else {
                fullscreen_fixed_radio->setSelected(true);
            }
        } else {
            windowed_radio->setSelected(true);
        }
        
        char buf[128];
        snprintf(buf, 128, "%dx%d",
            cfg->queryInt("Game_xres", 1024),
            cfg->queryInt("Game_yres", 768));
        CEGUI::ListboxItem *res_item = resolutions_combo->findItemWithText(buf, NULL);
        if (!res_item) {
            res_item = new CEGUI::ListboxTextItem(buf);
            resolutions_combo->addItem(res_item);
        }
        resolutions_combo->setText(res_item->getText());
        res_item->setSelected(true);

        shaders_enabled_checkbox->setSelected(
            cfg->queryBool("Game_use_shaders"));
    }
    
    void SettingsDlg::saveToConfig() {
        cfg->set("Controls_enable_joystick",
            joystick_enabled_checkbox->isSelected()?"true":"false");
        if (windowed_radio->isSelected()) {
            cfg->set("Game_fullscreen", "false");
            cfg->set("Game_auto_resolution", "false");
        } else if (fullscreen_auto_radio->isSelected()) {
            cfg->set("Game_fullscreen", "true");
            cfg->set("Game_auto_resolution", "true");
        } else {
            cfg->set("Game_fullscreen", "true");
            cfg->set("Game_auto_resolution", "false");
        }
        
        int w,h;
        sscanf(resolutions_combo->getText().c_str(), "%dx%d", &w, &h);
        char wstr[32], hstr[32];
        snprintf(wstr, 32, "%d", w);
        snprintf(hstr, 32, "%d", h);
        cfg->set("Game_xres", wstr);
        cfg->set("Game_yres", hstr);

        cfg->set("Game_use_shaders",
            shaders_enabled_checkbox->isSelected()?"true":"false");
    }
    
    void SettingsDlg::loadEvents() {
        events_list->resetList();
        events_list->addColumn("Name", 0, CEGUI::UDim(1.0,0));
        
        typedef std::vector<std::string> Events;
        Events events = remapper->getActions();
        for (Events::iterator i= events.begin(); i!=events.end(); ++i) {
            events_list->addRow();
            CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem(*i);
            item->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush"); 
            events_list->setItem(item, 0, events_list->getRowCount()-1);
        }
    }
    
    void SettingsDlg::saveEvents() {
    }
    
    void SettingsDlg::loadButtons() {
        keyboard_key_button->setText("click to configure");
        mouse_button_button->setText("click to configure");
        joystick_button_button->setText("click to configure");

        // If no event selected, we're done
        if (0 == events_list->getSelectedCount()) {
            return;
        }
        
        // Load buttons for selected event
        CEGUI::ListboxItem * item = events_list->getFirstSelectedItem();
        typedef std::vector<EventRemapper::Button> Buttons;
        Buttons buttons =
            remapper->getButtonsForAction(item->getText().c_str());
        
        for(Buttons::iterator i=buttons.begin(); i != buttons.end(); ++i) {
            switch (i->type) {
            case EventRemapper::KEYBOARD_KEY:
                keyboard_key_button->setText(i->getFriendlyName().c_str());
                break;
            case EventRemapper::MOUSE_BUTTON:
                mouse_button_button->setText(i->getFriendlyName().c_str());
                break;
            case EventRemapper::JOYSTICK_BUTTON:
                joystick_button_button->setText(i->getFriendlyName().c_str());
                break;
            }
        }
    }
    
    void SettingsDlg::buttonDetected(EventRemapper::Button btn) {
        wait_for_keyboard_key_wnd->setVisible(false);
        wait_for_mouse_button_wnd->setVisible(false);
        wait_for_joystick_button_wnd->setVisible(false);
        
        remapper->popEventFilter();
        
        // If no event selected, we're done
        if (0 == events_list->getSelectedCount()) {
            return;
        }
        
        // Get name of selected event
        CEGUI::ListboxItem * item = events_list->getFirstSelectedItem();
        const char *event = item->getText().c_str();
        
        remapper->unmapButtonsOfType(btn.type, event);
        remapper->mapButton(btn, event);
        
        loadButtons();
    }
    
    void SettingsDlg::buttonDetectionCancelled() {
        wait_for_keyboard_key_wnd->setVisible(false);
        wait_for_mouse_button_wnd->setVisible(false);
        wait_for_joystick_button_wnd->setVisible(false);
        remapper->popEventFilter();
    }
    
} // namespace UI

