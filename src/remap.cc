#include <algorithm>
#include <sstream>
#include <tnl.h>
#include <remap.h>

SelectAxisByActivityTransform::SelectAxisByActivityTransform(
	float threshold, float init_value)
:	threshold(threshold), value(init_value), init(true)
{ }
	
float SelectAxisByActivityTransform::operator() (std::vector<float> & inputs) {
    if (inputs.size() != old_values.size()) {
        init = true;
    }
    
    if (init) {
	    old_values = inputs;
	    init = false;
    } else {
		for (int i=0; i<inputs.size(); ++i) {
			if (std::abs(inputs[i]-old_values[i]) > threshold) {
			    value = inputs[i];
			    break;
			}
		}
		old_values=inputs;
	}
    
    return value;
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
    case JOYSTICK_BUTTON: out << "button "; break;
    }
    
    switch(type) {
    case KEYBOARD_KEY: out << button << " (" << SDL_GetKeyName((SDLKey)button) << ")"; break;
    case MOUSE_BUTTON: out << button; break;
    case JOYSTICK_BUTTON: out << (button+1); break;
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

    JoystickAxisMap::iterator i = joystick_axis_map.find(jaxis);
    if (i != joystick_axis_map.end()) {
        controls->setFloat(i->second, value);
    }
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
    return controls->getFloat(axis, 0.0f);
}

void EventRemapper::setAxis(const char * axis, float val)
{
    controls->setFloat(axis, val);
}
