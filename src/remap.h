// remap.h
#ifndef REMAP_H
#define REMAP_H

#include <landscape.h>
#include <modules/engines/controls.h>

#include <SDL_events.h>

#include <vector>
#include <map>
#include <string>

struct AxisTransform : public Object {
    virtual float operator() (std::vector<float> & inputs) = 0;
};

class LinearAxisTransform : public AxisTransform {
    float a,b;
public:
    LinearAxisTransform(float a=1.0f, float b=0.0f) : a(a), b(b) { }
    virtual float operator() (std::vector<float> & inputs) { return a*inputs[0]+b; }
};

struct SumAxesTransform : public AxisTransform {
    virtual float operator() (std::vector<float> & inputs) {
        float out = 0.0f;
        for(int i=0; i<inputs.size(); i++) out += inputs[i];
        return out;
    }
};

class ClampAxisTransform : public AxisTransform {
    float a, b;
public:
    ClampAxisTransform(float a=-1.0f, float b=1.0f) : a(a), b(b) { }
    virtual float operator() (std::vector<float> & inputs) {
        return (inputs[0]<a)?a:((inputs[0]>b)?b:inputs[0]);
    }
};

class SelectAxisByActivityTransform : public AxisTransform {
	std::vector<float> old_values;
	float threshold, value;
	bool init;
public:
	SelectAxisByActivityTransform(float threshold = 0.0f, float init_value=0.0f);
	virtual float operator() (std::vector<float> & inputs);
};


struct AxisManipulator {
    std::vector<std::string> inputs;
    std::string output;
    Ptr<AxisTransform> transform;

    inline AxisManipulator(Ptr<AxisTransform> transform, std::string out)
    :   output(out), transform(transform)
    { }

    inline AxisManipulator & input(std::string in) {
        inputs.push_back(in);
        return *this;
    }
};

/// A means for stacked event filtering.
/// An event filter receives events, can perform actions upon them and then
/// decide whether the event should be passed to the next-higher stacked
/// event filter.
class IEventFilter : virtual public Object
{
public:
    /** Called whenever an event occurs.
        Can decide whether the event should be passed on.
        @param  ev      the event to dispatch
        @return true, if the event should be passed on, false otherwise
    */
    virtual bool feedEvent(SDL_Event & ev) = 0;
};


/// Enables events to trigger method calls.
/// The organization into sheets makes it possible to switch between
/// them. Each input context has a sheet associated with it.
/// For example, while the player controls a car, a different sheet
/// gets used as while he controls an airplane.
/// EventRemapper allows multiple sheets to be active at once. If an event
/// is triggered, only the latest-added sheet handling the event receives it.
class EventSheet : public Object
{
    typedef std::map<std::string, ActionSignal *>        OutMap;
    OutMap outmap;
public:
    ~EventSheet();
    void map(const char *action, const ActionSlot & slot);
    bool triggerAction(const char *action);
};

class EventRemapper : public SigObject
{
public:
    EventRemapper(IoState *state);
    ~EventRemapper();

    inline void map(const char *action, const ActionSlot & slot) {
        event_sheets[0]->map(action,slot);
    }
    
    void mapKey(int key, bool pressed, const char *action);
    void mapMouseButton(int button, bool pressed, const char *action);
    void mapJoystickButton(int js, int button, bool pressed, const char *action);

    void mapJoystickAxis(int js, int joyaxis,
        const char * axis);
    void mapRelativeMouseAxes(const char* x_axis, const char *y_axis);
    void mapAbsoluteMouseAxes(const char* x_axis, const char *y_axis);
    void addAxisManipulator(AxisManipulator & manip);

    float getAxis(const char * axis);
    void setAxis(const char * axis, float val);

    void pushEventFilter(Ptr<IEventFilter> filter);
    void popEventFilter();
    
    void addEventSheet(Ptr<EventSheet> sheet);
    void removeEventSheet(Ptr<EventSheet> sheet);

    void beginEvents();
    void feedEvent(SDL_Event & ev);
    void endEvents();

    /// Called within SDL event handling, but may be called by user too, to trigger a named action
    bool triggerAction(const char * action);
    /// Emitted within triggerAction()
    SigC::Signal1<void, const char *> sig_action_triggered;

    inline Ptr<Controls> getControls() { return controls; }

private:
    void keyEvent(SDL_KeyboardEvent & ev);
    void mouseButtonEvent(SDL_MouseButtonEvent & ev);
    void mouseMotionEvent(SDL_MouseMotionEvent & ev);
    void joyButtonEvent(SDL_JoyButtonEvent & ev);
    void joyAxisEvent(SDL_JoyAxisEvent & ev);

    //using namespace std;

    typedef int KeyCode;
    typedef int ButtonNumber;
    typedef int JoystickIndex;
    typedef int AxisIndex;
    typedef std::pair<KeyCode,                            bool> KeyState;
    typedef std::pair<ButtonNumber,                       bool> MouseButtonState;
    typedef std::pair<std::pair<JoystickIndex, ButtonNumber>, bool> 
        JoystickButtonState;

    typedef std::map<KeyState,              std::string> KeyMap;
    typedef std::map<MouseButtonState,      std::string> MouseButtonMap;
    typedef std::map<JoystickButtonState,   std::string> JoystickButtonMap;
 
    typedef std::pair<JoystickIndex, AxisIndex>          JoystickAxis;
    typedef std::map<JoystickAxis, std::string>          JoystickAxisMap;

    typedef std::vector<AxisManipulator>                 AxisManips;

    typedef std::vector<Ptr<IEventFilter> >              EventFilters;
    typedef std::vector<Ptr<EventSheet> >                EventSheets;

    std::string         abs_mouse_x, abs_mouse_y, rel_mouse_x, rel_mouse_y;
    float               x_accum, y_accum;
    KeyMap              keymap;
    MouseButtonMap      mouse_button_map;
    JoystickButtonMap   joystick_button_map;
    JoystickAxisMap     joystick_axis_map;
    Ptr<Controls>       controls;
    AxisManips axismanips;
    EventFilters event_filters;
    EventSheets event_sheets;
};

#endif
