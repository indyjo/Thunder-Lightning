// remap.h
#ifndef REMAP_H
#define REMAP_H

#include <landscape.h>
#include <SDL/SDL_events.h>

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

class EventRemapper : public SigC::Object
{
public:
    EventRemapper();
    ~EventRemapper();

    void mapKey(int key, bool pressed, const char *action);
    void mapMouseButton(int button, bool pressed, const char *action);
    void mapJoystickButton(int js, int button, bool pressed, const char *action);

    void map(const char *action, const ActionSlot & slot);

    void mapJoystickAxis(int js, int joyaxis,
        const char * axis);
    void mapRelativeMouseAxes(const char* x_axis, const char *y_axis);
    void mapAbsoluteMouseAxes(const char* x_axis, const char *y_axis);
    void addAxisManipulator(AxisManipulator & manip);

    float getAxis(const char * axis);

    void pushEventFilter(Ptr<IEventFilter> filter);
    void popEventFilter();

    void beginEvents();
    void feedEvent(SDL_Event & ev);
    void endEvents();

    void triggerAction(const char * action);

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
    typedef std::pair<std::pair<JoystickIndex, ButtonNumber>,
                      bool>                                     JoystickButtonState;

    typedef std::map<KeyState,              std::string> KeyMap;
    typedef std::map<MouseButtonState,      std::string> MouseButtonMap;
    typedef std::map<JoystickButtonState,   std::string> JoystickButtonMap;
    typedef std::map<std::string, ActionSignal *>        OutMap;

    typedef std::pair<JoystickIndex, AxisIndex>          JoystickAxis;
    typedef std::map<JoystickAxis, std::string>          JoystickAxisMap;
    typedef std::map<std::string, float>                 AxisMap;

    typedef std::vector<AxisManipulator>                 AxisManips;

    typedef std::vector<Ptr<IEventFilter> >              EventFilters;

    std::string         abs_mouse_x, abs_mouse_y, rel_mouse_x, rel_mouse_y;
    float               x_accum, y_accum;
    KeyMap              keymap;
    MouseButtonMap      mouse_button_map;
    JoystickButtonMap   joystick_button_map;
    JoystickAxisMap     joystick_axis_map;
    OutMap outmap;
    AxisMap axismap;
    AxisManips axismanips;
    EventFilters event_filters;
};

#endif
