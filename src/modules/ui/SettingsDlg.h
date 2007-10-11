#ifndef TNL_SETTINGSDLG_H
#define TNL_SETTINGSDLG_H

#include <interfaces/IConfig.h>
#include <CEGUI.h>
#include "Dialog.h"

class EventRemapper;

namespace UI {
    class MainGUI;
    
    class SettingsDlg : public Dialog
    {
    public:
        SettingsDlg(MainGUI& main_gui, Ptr<IConfig> cfg, Ptr<EventRemapper> r);
        
        virtual void enable(MainGUI & main_gui);
        virtual void disable(MainGUI & main_gui);
    private:
        bool back(const CEGUI::EventArgs &);
        bool customizeButtons(const CEGUI::EventArgs &);
        bool buttonsCustomized(const CEGUI::EventArgs &);
        bool eventSelected(const CEGUI::EventArgs &);
        bool clearButton(const CEGUI::EventArgs &);
        bool configureButton(const CEGUI::EventArgs &);
        
        // called by game's post_frame signal
        void onFrame();
        SigC::Signal1<void, float> tick;
        
        void loadFromConfig();
        void saveToConfig();
        
        void loadEvents();
        void saveEvents();
        void loadButtons();
        
        void buttonDetected(EventRemapper::Button);
        void buttonDetectionCancelled();
        
    private:
        MainGUI & main_gui;
        Ptr<IConfig> cfg;
        Ptr<EventRemapper> remapper;
        CEGUI::Window *root;
        CEGUI::Window *settings_wnd;
        CEGUI::Window *buttons_wnd;
        CEGUI::Window *wait_for_keyboard_key_wnd;
        CEGUI::Window *wait_for_mouse_button_wnd;
        CEGUI::Window *wait_for_joystick_button_wnd;
        CEGUI::Checkbox *joystick_enabled_checkbox;
        CEGUI::RadioButton *fullscreen_auto_radio;
        CEGUI::RadioButton *fullscreen_fixed_radio;
        CEGUI::RadioButton *windowed_radio;
        CEGUI::Combobox *resolutions_combo;
        CEGUI::Checkbox *shaders_enabled_checkbox;
        CEGUI::MultiColumnList *events_list;
        CEGUI::PushButton *keyboard_key_button;
        CEGUI::PushButton *keyboard_clear_button;
        CEGUI::PushButton *mouse_button_button;
        CEGUI::PushButton *mouse_clear_button;
        CEGUI::PushButton *joystick_button_button;
        CEGUI::PushButton *joystick_clear_button;
    };

} // namespace

#endif

