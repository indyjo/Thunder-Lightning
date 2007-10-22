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
        bool customizeAxes(const CEGUI::EventArgs &);
        bool axesCustomized(const CEGUI::EventArgs &);
        bool axisSelected(const CEGUI::EventArgs &);
        bool clearAxis(const CEGUI::EventArgs &);
        bool configureAxis(const CEGUI::EventArgs &);
        bool toggleInvertAxis(const CEGUI::EventArgs &);
        
        // called by game's post_frame signal
        void onFrame();
        SigC::Signal1<void, float> tick;
        
        void loadFromConfig();
        void saveToConfig();
        
        void loadEvents();
        void saveEvents();
        void loadButtons();
        
        void loadAxes();
        void saveAxes();
        void loadJoystickAxis();
        
        void buttonDetected(EventRemapper::Button);
        void axisDetected(EventRemapper::JoystickAxis);
        void detectionCancelled();
        
    private:
        MainGUI & main_gui;
        Ptr<IConfig> cfg;
        Ptr<EventRemapper> remapper;
        CEGUI::Window *root;
        CEGUI::Window *settings_wnd;
        CEGUI::Window *buttons_wnd;
        CEGUI::Window *axes_wnd;
        CEGUI::Window *wait_for_keyboard_key_wnd;
        CEGUI::Window *wait_for_mouse_button_wnd;
        CEGUI::Window *wait_for_joystick_button_wnd;
        CEGUI::Window *wait_for_joystick_axis_wnd;
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
        CEGUI::Window *event_description_label;
        CEGUI::MultiColumnList *axes_list;
        CEGUI::Window *axis_description_label;
        CEGUI::PushButton *joystick_axis_button;
        CEGUI::PushButton *joystick_axis_clear_button;
        CEGUI::Checkbox *joystick_axis_inverted_checkbox;
    };

} // namespace

#endif

