// ---------------------------------------------------------------
// |  ActorMapping                                               |
// ---------------------------------------------------------------


#include <interfaces/IActor.h>

#include "mappings.h"

template<>
Ptr<IActor> unwrapObject<Ptr<IActor> >(IoObject * self) {
	return (IActor*)self->data;
}

namespace {
	
	struct ActorMapping
	:	public TemplatedObjectMapping<IActor>
	{
		static void addMapping(Ptr<IGame> game, IoState * state) {
			IoObject *lobby = state->lobby;
			
			IoObject *self = proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(lobby, IOSTRING("Actor"), self);
		}
		
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"asPositionProvider",
					castfunc<Ptr<IActor>, Ptr<IPositionProvider> >},
				{"getNumViews", getNumViews},
				{"getRelativeDamage", getRelativeDamage},
				{"isAlive", isAlive},
				{"setControlMode", setControlMode},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			self->tag = tag(state, "Actor");
			self->data = 0;
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
		
		static IoObject *isAlive(IoObject *self, IoObject*locals, IoObject*m) {
			BEGIN_FUNC("Actor.isAlive")
			if (getObject(self)->getState() == IActor::ALIVE) {
				return self;
			} else {
				return IONIL(self);
			}
		}
		
		SET_ENUM(setControlMode, IActor::ControlMode)
		GET_NUMBER(getNumViews)
		GET_NUMBER(getRelativeDamage)
	};
} // namespace

template<>
void addMapping<IActor>(Ptr<IGame> game, IoState *state) {
	ActorMapping::addMapping(game,state);
}

template<>
IoObject * wrapObject<Ptr<IActor> >(Ptr<IActor> actor, IoState *state) {
	IoObject *new_object = ActorMapping::rawClone(
		IoObject_getSlot_(
			state->lobby,
			IoState_stringWithCString_(state, "Actor")));
	new_object->data = &*actor;
	return new_object;
}

