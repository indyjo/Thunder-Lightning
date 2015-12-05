#include <algorithm>
#include <sstream>
#include <tnl.h>
#include <AxisTransform.h>
#include <remap.h>

AxisManipulator::AxisManipulator(Ptr<AxisTransform> transform, std::string out)
:   output(out), transform(transform)
{ }

AxisManipulator & AxisManipulator::input(std::string in) {
    inputs.push_back(in);
    return *this;
}

EventSheet::~EventSheet() {
    // Delete every ActionSignal instance from the out map
    for(OutMap::iterator i=outmap.begin(); i!=outmap.end(); i++)
        delete i->second;
}

void EventSheet::map(const char *action, const ActionSlot & slot)
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

bool EventSheet::triggerAction(const char * action)
{
    OutMap::iterator o = outmap.find( action );
    if (o != outmap.end()) {
        //ls_message("emitting action %s.\n", action);
        o->second->emit();
        return true;
    }
    //ls_warning("EventSheet::triggerAction('%s') : no handler found.\n", action);
    return false;
}

std::string EventRemapper::Button::getFriendlyName() const {
    std::ostringstream out;
    
    switch(type) {
    case KEYBOARD_KEY: if (device != 0 ) out << "keyboard "; break;
    case MOUSE_BUTTON: out << "mouse "; break;
    case JOYSTICK_BUTTON: out << "joystick "; break;
    }

    if (device != 0) {
        out << "#" << (device+1) << " ";
    }
        
    switch(type) {
    case KEYBOARD_KEY: out << "key "; break;
    case MOUSE_BUTTON: out << "button "; break;
    case JOYSTICK_BUTTON: {
            if (button < 256) {
                out << "button ";
            } else {
                out << "hat ";
                if (button >= 260) {
                    // coolie hat number > 1
                    out << ((button-256)/4)+1 << " ";
                }
            }
            break;
        }
    }
    
    switch(type) {
    case KEYBOARD_KEY: out << button << " (" << SDL_GetKeyName((SDLKey)button) << ")"; break;
    case MOUSE_BUTTON: out << button; break;
    case JOYSTICK_BUTTON: {
            if (button < 256) {
                out << (button+1);
            } else {
                int direction = (button-256) % 4;
                switch (direction) {
                    case 0: out << "up"; break;
                    case 1: out << "right"; break;
                    case 2: out << "down"; break;
                    case 3: out << "left"; break;
                }
            }
        }
    }
    
    return out.str();
}

EventRemapper::EventRemapper()
{
    controls = new DataNode;
    // default sheet
    event_sheets.push_back(new EventSheet());
}

EventRemapper::~EventRemapper()
{
}

void EventRemapper::clearButtonMappings() {
    button_map.clear();
}

void EventRemapper::clearEventFilters() {
    event_filters.clear();
}

void EventRemapper::clearAxisManipulators() {
    axismanips.clear();
}

void EventRemapper::clearJoystickAxisMappings() {
    joystick_axis_map.clear();
}

std::vector<std::string> EventRemapper::getActions() {
    typedef std::vector<std::string> Actions;
    Actions actions;
    
    for(ButtonMap::iterator i=button_map.begin(); i!=button_map.end(); ++i) {
        actions.push_back(i->second);
    }
    
    std::sort(actions.begin(), actions.end());
    Actions::iterator new_end = std::unique(actions.begin(), actions.end());
    actions.resize(new_end - actions.begin());
    
    return actions;
}

std::vector<EventRemapper::Button> EventRemapper::getButtonsForAction(const char *action) {
    std::vector<Button> buttons;
    for(ButtonMap::iterator i=button_map.begin(); i!=button_map.end(); ++i) {
        if (i->second == action) {
            buttons.push_back(i->first);
        }
    }
    
    std::sort(buttons.begin(), buttons.end());
    return buttons;
}

std::vector<EventRemapper::JoystickAxis> EventRemapper::getJoystickAxesForAxis(const char *axis) {
    std::vector<JoystickAxis> axes;
    
    for(JoystickAxisMap::iterator i=joystick_axis_map.begin(); i!=joystick_axis_map.end(); ++i) {
        if (i->second == axis) {
            axes.push_back(i->first);
        }
    }
    
    return axes;
}

void EventRemapper::mapButton(const EventRemapper::Button& btn, const char *action) {
    button_map.insert(std::make_pair(btn, action));
}

void EventRemapper::unmapButtonsOfType(EventRemapper::ButtonType type, const char *action) {
    ButtonMap::iterator i=button_map.begin();
    while (i!=button_map.end()) {
        if (i->first.type == type && i->second == action) {
            // found a mapped button of the sought after type.
            // increment and erase
            button_map.erase( i++ );
        } else {
            // just increment
            ++i;
        }
    }
}

void EventRemapper::pushEventFilter(Ptr<IEventFilter> filter) {
    event_filters.push_back(filter);
}

void EventRemapper::popEventFilter() {
    ls_message("Pop event filter.\n");
    if (!event_filters.empty())
        event_filters.pop_back();
}

void EventRemapper::addEventSheet(Ptr<EventSheet> sheet) {
    event_sheets.push_back(sheet);
}

void EventRemapper::removeEventSheet(Ptr<EventSheet> sheet) {
    //ls_message("EventRemapper::removeEventSheet(%p)\n", &*sheet);
    //ls_message("  event_sheets.size(): %d\n", event_sheets.size());
    //ls_message("  found sheet:%p\n",&**std::find(event_sheets.begin(), event_sheets.end(), sheet));
    event_sheets.erase(std::find(event_sheets.begin(), event_sheets.end(), sheet));
    //ls_message("  event_sheets.size(): %d\n", event_sheets.size());
}


void EventRemapper::beginEvents() {
    x_accum = y_accum = 0.0f;
}

void EventRemapper::endEvents() {
    controls->setFloat(rel_mouse_x, x_accum);
    controls->setFloat(rel_mouse_y, y_accum);
    std::vector<float> inputs;
    for(int i=0; i<axismanips.size(); i++) {
        inputs.resize(axismanips[i].inputs.size());
        for(int j=0; j<axismanips[i].inputs.size(); j++) {
            inputs[j] = getAxis(axismanips[i].inputs[j].c_str());
        }
        float output = (*axismanips[i].transform) (inputs);
        controls->setFloat(axismanips[i].output, output);
    }
}

void EventRemapper::feedEvent(SDL_Event & ev)
{
    // First pass the event to the filters (used for dialogs).
    // As filters might de-register themselves, copy the list first.
    // Iterate from back to front so that the most-recently-pushed filter
    // gets the first chance to handle the event.
    if (!event_filters.empty()) {
        std::vector<Ptr<IEventFilter> > event_filters_copy = event_filters;
        for(int i=event_filters_copy.size()-1; i>=0; i--)
            if (!event_filters_copy[i]->feedEvent(ev))
                return;
    }

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
    case SDL_JOYHATMOTION:
        joyHatEvent(ev.jhat);
        break;
    default:
        break;
    }
}


bool EventRemapper::triggerAction(const char * action)
{
    sig_action_triggered.emit(action);

    typedef EventSheets::reverse_iterator Iter;
    for (Iter i=event_sheets.rbegin(); i!=event_sheets.rend(); ++i) {
        if ((*i)->triggerAction(action)) return true;
    }
    //ls_warning("EventRemapper::triggerAction('%s'):\n"
    //           "  could not deliver event to any event sheet.\n"
    //           "  event_sheets %s empty.\n",
    //           action, event_sheets.empty()?"is":"is not");
    
    return false;
}

void EventRemapper::keyEvent(SDL_KeyboardEvent & ev)
{
    int key = ev.keysym.sym;
    bool pressed = (ev.state == SDL_PRESSED);
    
    buttonEvent(Button(KEYBOARD_KEY, 0, key), pressed);
}

void EventRemapper::mouseButtonEvent(SDL_MouseButtonEvent & ev)
{
    int button = ev.button;
    bool pressed = (ev.state == SDL_PRESSED);
    
    buttonEvent(Button(MOUSE_BUTTON, 0, button), pressed);
}

void EventRemapper::mouseMotionEvent(SDL_MouseMotionEvent & ev)
{
    x_accum += ev.xrel;
    y_accum += ev.yrel;
    controls->setFloat(abs_mouse_x, ev.x);
    controls->setFloat(abs_mouse_y, ev.y);
}


void EventRemapper::joyButtonEvent(SDL_JoyButtonEvent & ev)
{
    int joy = ev.which;
    int button = ev.button;
    bool pressed = (ev.state == SDL_PRESSED);
    
    buttonEvent(Button(JOYSTICK_BUTTON, joy, button), pressed);
}

void EventRemapper::joyAxisEvent(SDL_JoyAxisEvent & ev)
{
    JoystickAxis jaxis(ev.which, ev.axis);
    float value = (float) ev.value / 32768.0f;

    /*
    ls_message("Joystick %d axis %d: %f.\n",
        jaxis.first,
        jaxis.second,
        value);
    */

    typedef JoystickAxisMap::iterator Iter;
    typedef std::pair<Iter,Iter> IterPair;
    IterPair iters=joystick_axis_map.equal_range(jaxis);
    for(Iter i=iters.first; i!= iters.second; ++i) {
        float val = value;
        if (i->second[0] == '-') {
            val = -val;
        }
        controls->setFloat(i->second.substr(1), val);
    }
}

void EventRemapper::joyHatEvent(SDL_JoyHatEvent & ev)
{
    int joy = ev.which;
    int hat = ev.hat;
    
    Uint8 old_state = hat_positions[JoystickHat(joy,hat)];
    for (int i=0; i<4; ++i) {
        // Iterate through the four bits of the hat position, comparing
        // every bit with the corresponding bit of the saved state.
        Uint8 mask = 1<<i;
        if ((ev.value&mask) != (old_state&mask)) {
            // There was a change in this bit.
            // Generate a press or release event.
            buttonEvent(Button(JOYSTICK_BUTTON, joy, 256+4*hat+i), ev.value&mask);
        }
    }
    // save the hat position
    hat_positions[JoystickHat(joy,hat)] = ev.value;
}

void EventRemapper::buttonEvent(const Button & btn, bool pressed) {
    typedef ButtonMap::iterator Iter;
    std::pair<Iter, Iter> range = button_map.equal_range(btn);
    if (pressed) {
        // For button press events, trigger all actions mapped to the button
        for(Iter i=range.first; i!=range.second; ++i) {
            triggerAction(i->second.c_str());
        }
    } else {
        // For button release events, trigger only those mapped actions which
        // begin with a '+' character, substituting the '+' with a '-' (minus).
        for(Iter i=range.first; i!=range.second; ++i) {
            if (i->second[0] == '+') {
                std::string action = i->second;
                action[0] = '-';
                triggerAction(action.c_str());
            }
        }
    }
    
    if (pressed && range.first == range.second) {
        ls_message("EventRemapper: Unmapped %s pressed.\n", btn.getFriendlyName().c_str());
    }
}

void EventRemapper::mapJoystickAxis(int js, int joyaxis,
        const char * axis)
{
    ls_message("EventRemapper: Joystick %d axis %d mapped to %s.\n", js, joyaxis, axis);
    if (axis[0] != '+' && axis[0] != '-') {
        std::string plus_axis = std::string("+") + axis;
        joystick_axis_map.insert( std::make_pair(JoystickAxis(js, joyaxis), plus_axis) );
    } else {
        joystick_axis_map.insert( std::make_pair(JoystickAxis(js, joyaxis), axis) );    
    }
}

void EventRemapper::unmapJoystickAxes(const char *axis) {
    JoystickAxisMap::iterator i=joystick_axis_map.begin();
    while (i!=joystick_axis_map.end()) {
        if (i->second == axis) {
            // increment and erase
            joystick_axis_map.erase( i++ );
        } else {
            // just increment
            ++i;
        }
    }
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

void EventRemapper::addAxisWithDefaultHandling(const char *axis, bool nonnegative) {
    std::string v_axis = std::string("v_") + axis;
    std::string mouse_axis = std::string("mouse_") + axis;
    std::string mouse2_axis = std::string("mouse2_") + axis;
    std::string js_axis = std::string("js_") + axis;
    std::string js2_axis = std::string("js2_") + axis;
    
    // Set all input axis values to 0
    setAxis(v_axis.c_str(), 0);
    setAxis(mouse_axis.c_str(), 0);
    setAxis(js_axis.c_str(), 0);
    
    // Apply usual transformations on joystick and mouse values
    //FIXME: Make these values user-defined
    addAxisManipulator(
        AxisManipulator(new LinearAxisTransform(1.0f/5, 0.0f), mouse2_axis)
        .input(mouse_axis));
    addAxisManipulator(
        AxisManipulator(new JoystickAxisTransform(nonnegative, false, 0.01f, 0.04f), js2_axis)
        .input(js_axis));
    
    // Copy value from axis with most recent activity
    addAxisManipulator(
        AxisManipulator(new SelectAxisByActivityTransform(0.025f), axis)
        .input(v_axis)
        .input(mouse2_axis)
        .input(js2_axis));
        
    // Clamp the result to the value domain
    addAxisManipulator(
        AxisManipulator(new ClampAxisTransform(nonnegative?0.0f:-1.0f, 1.0f), axis)
        .input(axis));
}

float EventRemapper::getAxis(const char * axis)
{
    return controls->getFloat(axis, 0.0f);
}

void EventRemapper::setAxis(const char * axis, float val)
{
    controls->setFloat(axis, val);
}
