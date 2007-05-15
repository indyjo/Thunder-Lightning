// ---------------------------------------------------------------
// |  EventMapping                                               |
// ---------------------------------------------------------------

#include <remap.h>
#include "mappings.h"

namespace {
	
	struct EventMapping
	:	public TemplatedObjectMapping<EventRemapper>
	{
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject *self = proto(state);
			retarget(self, ptr(thegame->getEventRemapper()));
			IoState_registerProtoWithFunc_(state, self, proto);
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
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "EventRemapper"));
			IoObject_setDataPointer_(self, 0);
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		CREATE_FUNC(EventRemapper)
		
		static IoObject * mapKey
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("EventRemapper.mapKey")
			IOASSERT(IoMessage_argCount(m) == 3,"Expected 3 arguments")
			int key = IoMessage_locals_intArgAt_(m, locals, 0);
			bool pressed = !ISNIL(IoMessage_locals_valueArgAt_(m, locals, 1));
			char * action = IoMessage_locals_cStringArgAt_(m, locals, 2);
			getObject(self)->mapKey(key,pressed,action);
			return self;
		}

		static IoObject * mapMouseButton
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("EventRemapper.mapMouseButton")
			IOASSERT(IoMessage_argCount(m) == 3,"Expected 3 arguments")
			int button = IoMessage_locals_intArgAt_(m, locals, 0);
			bool pressed = !ISNIL(IoMessage_locals_valueArgAt_(m, locals, 1));
			char * action = IoMessage_locals_cStringArgAt_(m, locals, 2);
			getObject(self)->mapMouseButton(button,pressed,action);
			return self;
		}
		
		static IoObject * mapJoystickButton
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("EventRemapper.mapJoystickButton")
			IOASSERT(IoMessage_argCount(m) == 4,"Expected 4 arguments")
			int js = IoMessage_locals_intArgAt_(m, locals, 0);
			int button = IoMessage_locals_intArgAt_(m, locals, 1);
			bool pressed = !ISNIL(IoMessage_locals_valueArgAt_(m, locals, 2));
			char * action = IoMessage_locals_cStringArgAt_(m, locals, 3);
			getObject(self)->mapJoystickButton(js,button,pressed,action);
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
	};
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
	return (EventRemapper*)IoObject_dataPointer(self);
}

