#include <landscape.h>

EventRemapper::EventRemapper()
{
}

EventRemapper::~EventRemapper()
{
    // Delete every ActionSignal instance from the out map
    for(OutMap::iterator i=outmap.begin(); i!=outmap.end(); i++)
        delete i->second;
}

void EventRemapper::mapKey(int key, bool pressed, const char *action)
{
    keymap[KeyState(key, pressed)] = action;
}

void EventRemapper::mapMouseButton(int button, bool pressed, const char *action)
{
    mouse_button_map[MouseButtonState(button, pressed)] = action;
}

void EventRemapper::mapJoystickButton(int js, int button, bool pressed, const char *action)
{
    joystick_button_map[JoystickButtonState(std::make_pair(js,button), pressed)]
        = action;
}

void EventRemapper::map(const char *action, const ActionSlot & slot)
{
    OutMap::iterator i = outmap.find(action);
    ActionSignal *sig;
    if (i == outmap.end()) {
        sig = new ActionSignal;
        outmap[action] = sig;
    } else {
        sig = i->second;
    }

    sig->connect(slot);
}

void EventRemapper::pushEventFilter(Ptr<IEventFilter> filter) {
    event_filters.push_back(filter);
}

void EventRemapper::popEventFilter() {
    if (!event_filters.empty())
        event_filters.pop_back();
}

void EventRemapper::beginEvents() {
    x_accum = y_accum = 0.0f;
}

void EventRemapper::endEvents() {
    axismap[rel_mouse_x] = x_accum;
    axismap[rel_mouse_y] = y_accum;
    std::vector<float> inputs;
    for(int i=0; i<axismanips.size(); i++) {
        inputs.resize(axismanips[i].inputs.size());
        for(int j=0; j<axismanips[i].inputs.size(); j++) {
            inputs[j] = getAxis(axismanips[i].inputs[j].c_str());
        }
        float output = (*axismanips[i].transform) (inputs);
        axismap[axismanips[i].output] = output;
    }
}

void EventRemapper::feedEvent(SDL_Event & ev)
{
    // First pass the event to the filters
    for(int i=event_filters.size()-1; i>=0; i--)
        if (!event_filters[i]->feedEvent(ev))
            return;

    switch(ev.type) {
    case SDL_KEYUP:
    case SDL_KEYDOWN:
        keyEvent(ev.key);
        break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        mouseButtonEvent(ev.button);
        break;
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
        joyButtonEvent(ev.jbutton);
        break;
    case SDL_MOUSEMOTION:
        mouseMotionEvent(ev.motion);
        break;
    case SDL_JOYAXISMOTION:
        joyAxisEvent(ev.jaxis);
        break;
    default:
        break;
    }
}


void EventRemapper::triggerAction(const char * action)
{
    OutMap::iterator o = outmap.find( action );
    if (o != outmap.end()) {
        o->second->emit();
    }
}

void EventRemapper::keyEvent(SDL_KeyboardEvent & ev)
{
    int key = ev.keysym.sym;
    bool pressed = (ev.state == SDL_PRESSED);
    KeyState ks(key, pressed);

    KeyMap::iterator i = keymap.find(ks);
    if (i != keymap.end())
        triggerAction(i->second.c_str());
}

void EventRemapper::mouseButtonEvent(SDL_MouseButtonEvent & ev)
{
    int button = ev.button;
    bool pressed = (ev.state == SDL_PRESSED);
    MouseButtonState bs(button, pressed);

    ls_message("Mouse button %d %s.\n",
        button,
        pressed?"pressed":"released");

    MouseButtonMap::iterator i = mouse_button_map.find(bs);
    if (i != mouse_button_map.end())
        triggerAction(i->second.c_str());
}

void EventRemapper::mouseMotionEvent(SDL_MouseMotionEvent & ev)
{
    ls_message("Mouse motion: %d (%+d) %d (%+d)\n",
        ev.x, ev.xrel, ev.y, ev.yrel);
    x_accum += ev.xrel;
    y_accum += ev.yrel;
    axismap[abs_mouse_x] = ev.x;
    axismap[abs_mouse_y] = ev.y;
}


void EventRemapper::joyButtonEvent(SDL_JoyButtonEvent & ev)
{
    int joy = ev.which;
    int button = ev.button;
    bool pressed = (ev.state == SDL_PRESSED);
    JoystickButtonState js(std::make_pair(joy, button), pressed);

    ls_message("Joystick %d button %d %s.\n",
        joy,
        button,
        pressed?"pressed":"released");

    JoystickButtonMap::iterator i = joystick_button_map.find(js);
    if (i != joystick_button_map.end())
        triggerAction(i->second.c_str());
}

void EventRemapper::joyAxisEvent(SDL_JoyAxisEvent & ev)
{
    JoystickAxis jaxis(ev.which, ev.axis);
    float value = (float) ev.value / 32768.0f;

    ls_message("Joystick %d axis %d: %f.\n",
        jaxis.first,
        jaxis.second,
        value);

    JoystickAxisMap::iterator i = joystick_axis_map.find(jaxis);
    if (i != joystick_axis_map.end()) {
        axismap[i->second] = value;
    }
}


void EventRemapper::mapJoystickAxis(int js, int joyaxis,
        const char * axis)
{
    joystick_axis_map[ JoystickAxis(js, joyaxis) ] = axis;
}

void EventRemapper::mapRelativeMouseAxes(const char* x_axis, const char *y_axis) {
    rel_mouse_x = x_axis;
    rel_mouse_y = y_axis;
}

void EventRemapper::mapAbsoluteMouseAxes(const char* x_axis, const char *y_axis) {
    abs_mouse_x = x_axis;
    abs_mouse_y = y_axis;
}

void EventRemapper::addAxisManipulator(AxisManipulator & manip) {
    axismanips.push_back(manip);
}

float EventRemapper::getAxis(const char * axis)
{
    AxisMap::iterator i = axismap.find(axis);
    if (i!=axismap.end()) return i->second;
    else return 0.0f;
}

