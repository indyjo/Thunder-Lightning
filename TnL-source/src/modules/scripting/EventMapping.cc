// ---------------------------------------------------------------
// |  EventMapping                                               |
// ---------------------------------------------------------------

#include <remap.h>
#include "mappings.h"

namespace {
	
	struct EventMapping
	:	public TemplatedObjectMapping<EventRemapper>
	{
        static const char *const id;
        
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject *self = proto(state);
			retarget(self, ptr(thegame->getEventRemapper()));
			IoState_registerProtoWithId_(state, self, id);
			IoObject_setSlot_to_(
				state->lobby, IOSYMBOL("EventRemapper"), self);
		}
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"mapKey", mapKey},
				{"mapMouseButton", mapMouseButton},
				{"mapJoystickButton", mapJoystickButton},
				{"mapJoystickAxis", mapJoystickAxis},
				{"mapRelativeMouseAxes", mapRelativeMouseAxes},
				{"mapAbsoluteMouseAxes", mapAbsoluteMouseAxes},
                {"controls", controls},
                {"trigger", triggerAction},
                {"actions", actions},
                {"clearButtonMappings", clearButtonMappings},
                {"clearJoystickAxisMappings", clearJoystickAxisMappings},
                {"registerAction", registerAction},
                {"registerAxis", registerAxis},
                {"registeredAxes", getRegisteredAxes},
                {"joystickAxesForAxis", getJoystickAxesForAxis},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "EventRemapper"));
			IoObject_setDataPointer_(self, 0);
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		CREATE_FUNC(EventRemapper, id)
		
		static IoObject * mapKey
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("EventRemapper.mapKey")
			IOASSERT(IoMessage_argCount(m) == 2,"Expected 2 arguments")
			int key = IoMessage_locals_intArgAt_(m, locals, 0);
			char * action = IoMessage_locals_cStringArgAt_(m, locals, 1);
			getObject(self)->mapButton(
			    EventRemapper::Button(EventRemapper::KEYBOARD_KEY, 0, key),
			    action);
			return self;
		}

		static IoObject * mapMouseButton
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("EventRemapper.mapMouseButton")
			IOASSERT(IoMessage_argCount(m) == 2,"Expected 2 arguments")
			int button = IoMessage_locals_intArgAt_(m, locals, 0);
			char * action = IoMessage_locals_cStringArgAt_(m, locals, 1);
			getObject(self)->mapButton(
			    EventRemapper::Button(EventRemapper::MOUSE_BUTTON, 0, button),
			    action);
			return self;
		}
		
		static IoObject * mapJoystickButton
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("EventRemapper.mapJoystickButton")
			IOASSERT(IoMessage_argCount(m) == 3,"Expected 3 arguments")
			int js = IoMessage_locals_intArgAt_(m, locals, 0);
			int button = IoMessage_locals_intArgAt_(m, locals, 1);
			char * action = IoMessage_locals_cStringArgAt_(m, locals, 2);
			getObject(self)->mapButton(
			    EventRemapper::Button(EventRemapper::JOYSTICK_BUTTON, js, button),
			    action);
			return self;
		}
		
		static IoObject * mapJoystickAxis
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("EventRemapper.mapJoystickAxis")
			IOASSERT(IoMessage_argCount(m) == 3,"Expected 3 arguments")
			int js = IoMessage_locals_intArgAt_(m, locals, 0);
			int jaxis = IoMessage_locals_intArgAt_(m, locals, 1);
			char * axis = IoMessage_locals_cStringArgAt_(m, locals, 2);
			getObject(self)->mapJoystickAxis(js,jaxis,axis);
			return self;
		}
		
		static IoObject * mapRelativeMouseAxes
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("EventRemapper.mapRelativeMouseAxes")
			IOASSERT(IoMessage_argCount(m) == 2,"Expected 2 arguments")
			char * axis_x = IoMessage_locals_cStringArgAt_(m, locals, 0);
			char * axis_y = IoMessage_locals_cStringArgAt_(m, locals, 1);
			getObject(self)->mapRelativeMouseAxes(axis_x, axis_y);
			return self;
		}
		
		static IoObject * mapAbsoluteMouseAxes
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("EventRemapper.mapAbsoluteMouseAxes")
			IOASSERT(IoMessage_argCount(m) == 2,"Expected 2 arguments")
			char * axis_x = IoMessage_locals_cStringArgAt_(m, locals, 0);
			char * axis_y = IoMessage_locals_cStringArgAt_(m, locals, 1);
			getObject(self)->mapAbsoluteMouseAxes(axis_x, axis_y);
			return self;
		}

        static IoObject * controls
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("EventRemapper.controls")
			return wrapObject<Ptr<DataNode> >(
			    getObject(self)->getControls(),
			    IOSTATE);
		}
		
        static IoObject * triggerAction
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("EventRemapper.trigger")
			char * action = IoMessage_locals_cStringArgAt_(m, locals, 0);
			return wrapObject<bool>(
			    getObject(self)->triggerAction(action),
			    IOSTATE);
		}
		
        /// Returns a list of objects with slots "name" and "buttons". A buttons
        /// slot is an object with symbol slot "type" and number slots
        /// "device" and "button"
        static IoObject *actions(IoObject *self, IoObject*locals, IoObject*m) {
            BEGIN_FUNC("EventRemapper.actions")
            typedef std::vector<std::string> Actions;
            Actions actions = getObject(self)->getActions();
            IoObject* actions_list = IoList_new(IOSTATE);
            for (Actions::iterator i = actions.begin(); i!= actions.end(); ++i) {
                IoObject* action_obj = IoObject_new(IOSTATE);
                IoObject_setSlot_to_(action_obj, IOSYMBOL("name"), IOSYMBOL(i->c_str()));
                
                typedef std::vector<EventRemapper::Button> Buttons;
                Buttons buttons = getObject(self)->getButtonsForAction(i->c_str());
                
                IoObject* buttons_list = IoList_new(IOSTATE);
                IoObject_setSlot_to_(action_obj, IOSYMBOL("buttons"), buttons_list);
                
                for(Buttons::iterator j=buttons.begin(); j!= buttons.end(); ++j) {
                    IoObject* button_obj = IoObject_new(IOSTATE);
                    
                    switch(j->type) {
                    case EventRemapper::KEYBOARD_KEY:
                        IoObject_setSlot_to_(button_obj, IOSYMBOL("type"), IOSYMBOL("KEYBOARD_KEY"));
                        break;
                    case EventRemapper::MOUSE_BUTTON:
                        IoObject_setSlot_to_(button_obj, IOSYMBOL("type"), IOSYMBOL("MOUSE_BUTTON"));
                        break;
                    case EventRemapper::JOYSTICK_BUTTON:
                        IoObject_setSlot_to_(button_obj, IOSYMBOL("type"), IOSYMBOL("JOYSTICK_BUTTON"));
                        break;
                    }
                    
                    IoObject_setSlot_to_(button_obj, IOSYMBOL("device"), IONUMBER(j->device));
                    IoObject_setSlot_to_(button_obj, IOSYMBOL("button"), IONUMBER(j->button));
                    
                    IoList_rawAppend_(buttons_list, button_obj);
                }
                
                IoList_rawAppend_(actions_list, action_obj);
            }
            return actions_list;
        }
        
        VOID_FUNC(clearButtonMappings)
        VOID_FUNC(clearJoystickAxisMappings)

        static IoObject * registerAction
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("EventRemapper.registerAction")
			IOASSERT(IoMessage_argCount(m) == 3,"Expected 3 arguments")
			std::string name = IoMessage_locals_cStringArgAt_(m, locals, 0),
			            friendly_name = IoMessage_locals_cStringArgAt_(m, locals, 1),
			            description = IoMessage_locals_cStringArgAt_(m, locals, 2);
			getObject(self)->action_dict[name] = EventRemapper::DictionaryEntry(
			    friendly_name, description);
			return self;
		}

        static IoObject * registerAxis
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("EventRemapper.registerAxis")
			IOASSERT(IoMessage_argCount(m) == 3,"Expected 3 arguments")
			std::string name = IoMessage_locals_cStringArgAt_(m, locals, 0),
			            friendly_name = IoMessage_locals_cStringArgAt_(m, locals, 1),
			            description = IoMessage_locals_cStringArgAt_(m, locals, 2);
			getObject(self)->axis_dict[name] = EventRemapper::DictionaryEntry(
			    friendly_name, description);
			return self;
		}
		
        static IoObject *getRegisteredAxes(IoObject *self, IoObject*locals, IoObject*m) {
            BEGIN_FUNC("EventRemapper.registeredAxes")
            
            IoList* axes_list = IoList_new(IOSTATE);
            
            typedef EventRemapper::Dictionary Dict;
            Dict & dict = getObject(self)->axis_dict;
            
            for(Dict::iterator i=dict.begin(); i!= dict.end(); ++i) {
                IoList_rawAppend_(axes_list, IOSYMBOL(i->first.c_str()));
            }
            
            return axes_list;
        }

		static IoObject * getJoystickAxesForAxis
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("EventRemapper.joystickAxesForAxis")
			IOASSERT(IoMessage_argCount(m) == 1,"Expected 1 argument")
			std::string name = IoMessage_locals_cStringArgAt_(m, locals, 0);
			
			typedef std::vector<EventRemapper::JoystickAxis> Axes;
			Axes axes = getObject(self)->getJoystickAxesForAxis(name.c_str());
			
			IoList* axes_list = IoList_new(IOSTATE);
			
			for(Axes::iterator i = axes.begin(); i!=axes.end(); ++i) {
			    IoObject * joyaxis = IoObject_new(IOSTATE);
			    IoObject_setSlot_to_(joyaxis, IOSYMBOL("joystick"), IONUMBER(i->first));
			    IoObject_setSlot_to_(joyaxis, IOSYMBOL("axis"), IONUMBER(i->second));
			    IoList_rawAppend_(axes_list, joyaxis);
			}
			
			return axes_list;
		}
	};
    
    const char *const EventMapping::id = "EventRemapper";
}

template<>
void addMapping<EventRemapper>(Ptr<IGame> game, IoState *state) {
	EventMapping::addMapping(game,state);
}

template<>
IoObject * wrapObject<Ptr<EventRemapper> >
(Ptr<EventRemapper> r, IoState *state) {
	return EventMapping::create(r,state);
}

template<>
Ptr<EventRemapper> unwrapObject<Ptr<EventRemapper> >(IoObject * self) {
    return EventMapping::getObject(self);
}

