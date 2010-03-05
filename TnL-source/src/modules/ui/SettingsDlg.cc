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
    class Detector : public IEventFilter, public SigC::Object
    {
    protected:
        float time_left;
    public:
        Detector()
            : time_left(8)
        { }

        virtual bool feedEvent(SDL_Event & ev)=0;

        void tick(float delta_t) {
            if (time_left > 0) {
                time_left -= delta_t;
                if (time_left <= 0) {
                    cancelled.emit();
                }
            }
        }
        
        SigC::Signal0<void> cancelled;
    };
    
    class ButtonDetector : public Detector
    {
        EventRemapper::ButtonType type;
    public:
        ButtonDetector(EventRemapper::ButtonType t)
            : type(t)
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
        
        SigC::Signal1<void, EventRemapper::Button> button_detected;

    private:
        void detect(EventRemapper::Button btn) {
            if ( btn.type == type ) {
                button_detected.emit(btn);
            }
        }
    };

    class JoystickAxisDetector : public Detector
    {
        typedef EventRemapper::JoystickAxis     Axis;
        typedef std::map<Axis, int>             Values;
        typedef std::map<Axis, unsigned int>    Motions;
        Values  last_values;
        Motions motions;
        
    public:
        virtual bool feedEvent(SDL_Event & ev) {
            if (time_left <= 0) {
                return false;
            }
            
            if (ev.type == SDL_JOYAXISMOTION) {
                Axis axis(ev.jaxis.which, ev.jaxis.axis);
                int value = ev.jaxis.value;
                if (last_values.find(axis) == last_values.end()) {
                    // first time we hear from this axis
                    last_values[axis] = value;
                } else {
                    // we already have a sample from last time, lets sum up the motion
                    int motion = std::abs(value - last_values[axis]);
                    if (motion + motions[axis] > 50000) {
                        axis_detected.emit(axis);
                    } else {
                        motions[axis] += motion;
                    }
                }
            }
            
            return false;
        }
        
        SigC::Signal1<void, EventRemapper::JoystickAxis> axis_detected;
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
        
        // back button and close handler
        settings_wnd->getChild("settingsWnd/backBtn")->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::back, this));
        settings_wnd->subscribeEvent(
            CEGUI::FrameWindow::EventCloseClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::back, this));
        
        // configure buttons and keys handler
        settings_wnd->getChild("settingsWnd/customizeEventsBtn")->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::customizeButtons, this));
        settings_wnd->getChild("settingsWnd/customizeJoystickBtn")->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::customizeAxes, this));


        ////////////////////////////////////////////////////////////////////
        // Buttons configuration window
        ////////////////////////////////////////////////////////////////////
        buttons_wnd = root->getChild("eventsWnd");
        events_list = (CEGUI::MultiColumnList *)
            buttons_wnd->getChild("eventsWnd/eventsList");
        events_list->setShowHorzScrollbar(false);
        events_list->setSelectionMode(CEGUI::MultiColumnList::RowSingle);
        events_list->subscribeEvent(
            CEGUI::MultiColumnList::EventSelectionChanged,
            CEGUI::SubscriberSlot(&SettingsDlg::eventSelected, this));

        reset_buttons_to_defaults_button = (CEGUI::PushButton *)
            buttons_wnd->getChild("eventsWnd/resetButton");
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
        
        event_description_label = buttons_wnd->getChild("eventsWnd/descriptionLabel");
            
        // react on "restore defaults" button
        reset_buttons_to_defaults_button->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::restoreDefaultButtons, this));
            
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
            
        // react on "OK" and close button
        buttons_wnd->getChild("eventsWnd/okButton")->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::buttonsCustomized, this));
        buttons_wnd->subscribeEvent(
            CEGUI::FrameWindow::EventCloseClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::buttonsCustomized, this));

        ////////////////////////////////////////////////////////////////////
        // Joystick axis window
        ////////////////////////////////////////////////////////////////////
        axes_wnd = root->getChild("axesWnd");
        axes_list = (CEGUI::MultiColumnList *)
            axes_wnd->getChild("axesWnd/axesList");
        axes_list->setShowHorzScrollbar(false);
        axes_list->setSelectionMode(CEGUI::MultiColumnList::RowSingle);
        axes_list->subscribeEvent(
            CEGUI::MultiColumnList::EventSelectionChanged,
            CEGUI::SubscriberSlot(&SettingsDlg::axisSelected, this));

        axis_description_label =
            axes_wnd->getChild("axesWnd/descriptionLabel");
        
        reset_axes_to_defaults_button = (CEGUI::PushButton *)
            axes_wnd->getChild("axesWnd/resetBtn");
        joystick_axis_button = (CEGUI::PushButton *)
            axes_wnd->getChild("axesWnd/joystickAxis");
        joystick_axis_clear_button = (CEGUI::PushButton *)
            axes_wnd->getChild("axesWnd/clearJoystickAxis");
        joystick_axis_inverted_checkbox = (CEGUI::Checkbox *)
            axes_wnd->getChild("axesWnd/invertAxisCheckbox");

        // react on "restore defaults" button
        reset_axes_to_defaults_button->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::restoreDefaultAxes, this));
        // react on "clear" button
        joystick_axis_clear_button->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::clearAxis, this));
        // react on "configure" button
        joystick_axis_button->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::configureAxis, this));
        // react on "invert this axis" toggle
        joystick_axis_inverted_checkbox->subscribeEvent(
            CEGUI::Checkbox::EventCheckStateChanged,
            CEGUI::SubscriberSlot(&SettingsDlg::toggleInvertAxis, this));
        // react on "OK" and close button
        axes_wnd->getChild("axesWnd/okButton")->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::axesCustomized, this));
        axes_wnd->subscribeEvent(
            CEGUI::FrameWindow::EventCloseClicked,
            CEGUI::SubscriberSlot(&SettingsDlg::axesCustomized, this));

        ////////////////////////////////////////////////////////////////////
        // Wait for ... windows
        ////////////////////////////////////////////////////////////////////
        wait_for_keyboard_key_wnd = root->getChild("waitForKeyboard");
        wait_for_mouse_button_wnd = root->getChild("waitForMouse");
        wait_for_joystick_button_wnd = root->getChild("waitForJoystick");
        wait_for_joystick_axis_wnd = root->getChild("waitForJoystickAxis");
        
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
            *this, &SettingsDlg::detectionCancelled));
        remapper->pushEventFilter(detector);
        
        return true;
    }
    
    bool SettingsDlg::restoreDefaultButtons(const CEGUI::EventArgs &) {
        Ptr<IGame> thegame = main_gui.getGame();
        if (thegame) {
            remapper->clearButtonMappings();
            
            IoState *state=thegame->getIoScriptingManager()->getMainState();
            IoState_doCString_(state, "EventRemapper loadDefaultButtons");
            
            loadEvents();
        }
        return true;
    }
    
    bool SettingsDlg::customizeAxes(const CEGUI::EventArgs &) {
        axes_wnd->setVisible(true);
        axes_wnd->setModalState(true);
        loadAxes();
        // Select the first item just to be sure
        if (axes_list->getRowCount() > 0) {
            axes_list->setItemSelectState(
                axes_list->getItemAtGridReference(CEGUI::MCLGridRef(0,0)),
                true);
        }
        return true;
    }

    bool SettingsDlg::axesCustomized(const CEGUI::EventArgs &) {
        axes_wnd->setVisible(false);
        axes_wnd->setModalState(false);
        return true;
    }

    bool SettingsDlg::axisSelected(const CEGUI::EventArgs &) {
        loadJoystickAxis();
        return true;
    }

    bool SettingsDlg::clearAxis(const CEGUI::EventArgs & args) {
        // If no axis selected, we're done
        if (0 == axes_list->getSelectedCount()) {
            return true;
        }
        
        // Get name of selected axis
        CEGUI::ListboxItem * item = axes_list->getFirstSelectedItem();
        const char *axis = item->getText().c_str();

        // Unmap all joystick axes mapped to that axis
        remapper->unmapJoystickAxes((std::string("+js_")+axis).c_str());
        remapper->unmapJoystickAxes((std::string("-js_")+axis).c_str());
        
        // Update
        loadJoystickAxis();
        
        return true;
    }

    bool SettingsDlg::configureAxis(const CEGUI::EventArgs & args) {
        wait_for_joystick_axis_wnd->setVisible(true);
        wait_for_joystick_axis_wnd->activate();

        Ptr<JoystickAxisDetector> detector = new JoystickAxisDetector;
        
        tick.connect(SigC::slot(
            *detector, &JoystickAxisDetector::tick));
        detector->axis_detected.connect( SigC::slot(
            *this, &SettingsDlg::axisDetected));
        detector->cancelled.connect( SigC::slot(
            *this, &SettingsDlg::detectionCancelled));
        remapper->pushEventFilter(detector);
        
        return true;
    }

    bool SettingsDlg::toggleInvertAxis(const CEGUI::EventArgs & args) {
        // If no axis selected, we're done
        if (0 == axes_list->getSelectedCount()) {
            return true;
        }
        
        // Get name of selected axis
        CEGUI::ListboxItem * item = axes_list->getFirstSelectedItem();
        std::string axis = item->getText().c_str();

        // Save axis mapping
        std::vector<EventRemapper::JoystickAxis> joystick_axes =
            remapper->getJoystickAxesForAxis(("+js_"+axis).c_str());
        if (joystick_axes.empty()) {
            joystick_axes = remapper->getJoystickAxesForAxis(("-js_"+axis).c_str());
        }
        
        if (joystick_axes.empty()) {
            // shouldn't happen, but let's play safe
            ls_warning("SettingsDlg: No joystick axis mapped to axis '%s'\n", axis.c_str());
            return true;
        }
        EventRemapper::JoystickAxis joyaxis = joystick_axes.front();
        
        // Unmap all joystick axes mapped to that axis
        remapper->unmapJoystickAxes(("+js_"+axis).c_str());
        remapper->unmapJoystickAxes(("-js_"+axis).c_str());
        
        // Remap, depending on checked state of "invert this axis" checkbox
        if (joystick_axis_inverted_checkbox->isSelected()) {
            remapper->mapJoystickAxis(joyaxis.first, joyaxis.second,
                                      ("-js_"+axis).c_str());
        } else {
            remapper->mapJoystickAxis(joyaxis.first, joyaxis.second,
                                      ("+js_"+axis).c_str());
        }
        
        // Update
        loadJoystickAxis();
        
        return true;
    }

    bool SettingsDlg::restoreDefaultAxes(const CEGUI::EventArgs &) {
        Ptr<IGame> thegame = main_gui.getGame();
        if (thegame) {
            remapper->clearJoystickAxisMappings();
            
            IoState *state=thegame->getIoScriptingManager()->getMainState();
            IoState_doCString_(state, "EventRemapper loadDefaultAxes");
            
            loadAxes();
        }
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
        events_list->addColumn("id", 0, CEGUI::UDim(0.4,0));
        events_list->addColumn("Name", 1, CEGUI::UDim(0.6,0));
        
        typedef std::vector<std::string> Events;
        Events events = remapper->getActions();
        for (Events::iterator i= events.begin(); i!=events.end(); ++i) {
            unsigned int row_idx = events_list->addRow();
            
            typedef EventRemapper::Dictionary Dict;
            
            std::string friendly_name = *i;
            Dict::iterator j = remapper->action_dict.find(*i);
            if (j != remapper->action_dict.end()) {
                friendly_name = j->second.first;
            }
            
            CEGUI::ListboxTextItem* item;
            
            item = new CEGUI::ListboxTextItem(*i);
            item->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush"); 
            events_list->setItem(item, 0, row_idx);

            item = new CEGUI::ListboxTextItem(friendly_name);
            item->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush"); 
            events_list->setItem(item, 1, row_idx);
        }

        events_list->setSortColumnByID(1);
        events_list->setSortDirection(CEGUI::ListHeaderSegment::None);
        events_list->setSortDirection(CEGUI::ListHeaderSegment::Descending);
    }
    
    void SettingsDlg::saveEvents() {
    }
    
    void SettingsDlg::loadButtons() {
        keyboard_key_button->setText("click to configure");
        mouse_button_button->setText("click to configure");
        joystick_button_button->setText("click to configure");
        event_description_label->setText("");

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
        
        // Load the event's description from the remapper's dictionary
        typedef EventRemapper::Dictionary::iterator Iter;
        Iter dict_entry = remapper->action_dict.find(item->getText().c_str());
        if (dict_entry != remapper->action_dict.end()) {
            event_description_label->setText(dict_entry->second.second);
        }
    }
    
    void SettingsDlg::loadAxes() {
        axes_list->resetList();
        axes_list->addColumn("id", 0, CEGUI::UDim(0.4,0));
        axes_list->addColumn("Name", 1, CEGUI::UDim(0.6,0));
        
        typedef EventRemapper::Dictionary Dict;
        Dict dict = remapper->axis_dict;
        
        for (Dict::iterator i=dict.begin(); i!=dict.end(); ++i) {
            std::string id = i->first;
            std::string name = i->second.first;
            
            unsigned int row_idx = axes_list->addRow();
            
            CEGUI::ListboxTextItem* item;
            
            item = new CEGUI::ListboxTextItem(id);
            item->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush"); 
            axes_list->setItem(item, 0, row_idx);

            item = new CEGUI::ListboxTextItem(name);
            item->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush"); 
            axes_list->setItem(item, 1, row_idx);
        }
        
        axes_list->setSortColumnByID(1);
        axes_list->setSortDirection(CEGUI::ListHeaderSegment::None);
        axes_list->setSortDirection(CEGUI::ListHeaderSegment::Descending);
    }
    
    void SettingsDlg::loadJoystickAxis() {
        joystick_axis_button->setText("click to configure");
        event_description_label->setText("");
        joystick_axis_inverted_checkbox->setEnabled(false);

        // If no event selected, we're done
        if (0 == axes_list->getSelectedCount()) {
            return;
        }
        
        // Load joystick axes mapped to selected axis
        CEGUI::ListboxItem * item = axes_list->getFirstSelectedItem();
        
        // First look for non-inverted ("+") axes
        typedef std::vector<EventRemapper::JoystickAxis> JoystickAxes;
        JoystickAxes joystick_axes = remapper->getJoystickAxesForAxis(
            (std::string("+js_") + item->getText()).c_str());
        
        bool axis_configured = false;
        EventRemapper::JoystickAxis configured_axis;
        
        if (!joystick_axes.empty()) {
            joystick_axis_inverted_checkbox->setEnabled(true);
            joystick_axis_inverted_checkbox->setSelected(false);
            configured_axis = joystick_axes.front();
            axis_configured = true;
        }
        
        /// now check for inverted axes
        joystick_axes = remapper->getJoystickAxesForAxis(
            (std::string("-js_") + item->getText()).c_str());

        if (!joystick_axes.empty()) {
            joystick_axis_inverted_checkbox->setEnabled(true);
            joystick_axis_inverted_checkbox->setSelected(true);
            configured_axis = joystick_axes.front();
            axis_configured = true;
        }
        
        if (axis_configured) {
            char buf[128];
            // pretty-print the joystick axis
            if (configured_axis.first == 0) {
                sprintf(buf, "Axis %d", configured_axis.second+1);
            } else {
                sprintf(buf, "Joystick %d axis %d", configured_axis.first+1, configured_axis.second+1);
            }
            joystick_axis_button->setText(buf);
        }
        
        // Load the axe's description from the remapper's dictionary
        typedef EventRemapper::Dictionary::iterator Iter;
        Iter dict_entry = remapper->axis_dict.find(item->getText().c_str());
        if (dict_entry != remapper->axis_dict.end()) {
            axis_description_label->setText(dict_entry->second.second);
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
    
    void SettingsDlg::axisDetected(EventRemapper::JoystickAxis joyaxis) {
        wait_for_joystick_axis_wnd->setVisible(false);
        
        remapper->popEventFilter();
        
        // If no axis selected, we're done
        if (0 == axes_list->getSelectedCount()) {
            return;
        }
        
        // Get name of selected axis
        CEGUI::ListboxItem * item = axes_list->getFirstSelectedItem();
        const char *axis = item->getText().c_str();
        ls_message("SettingsDlg: detected for '%s': stick %d axis %d\n", 
            axis, joyaxis.first, joyaxis.second);
        
        remapper->unmapJoystickAxes((std::string("+js_")+axis).c_str());
        remapper->unmapJoystickAxes((std::string("-js_")+axis).c_str());
        remapper->mapJoystickAxis(joyaxis.first, joyaxis.second, (std::string("+js_")+axis).c_str());
        
        loadJoystickAxis();
    }
    
    void SettingsDlg::detectionCancelled() {
        wait_for_keyboard_key_wnd->setVisible(false);
        wait_for_mouse_button_wnd->setVisible(false);
        wait_for_joystick_button_wnd->setVisible(false);
        wait_for_joystick_axis_wnd->setVisible(false);
        remapper->popEventFilter();
    }
    
} // namespace UI

