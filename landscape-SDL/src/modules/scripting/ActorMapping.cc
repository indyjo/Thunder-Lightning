// ---------------------------------------------------------------
// |  ActorMapping                                               |
// ---------------------------------------------------------------


#include <interfaces/IActor.h>
#include <interfaces/IProjectile.h>
#include <Faction.h>

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
				{"asMovementProvider",
					castfunc<Ptr<IActor>, Ptr<IMovementProvider> >},
				{"asPositionReceiver",
					castfunc<Ptr<IActor>, Ptr<IPositionReceiver> >},
				{"asMovementReceiver",
					castfunc<Ptr<IActor>, Ptr<IMovementReceiver> >},
				{"getNumViews", getNumViews},
				{"getRelativeDamage", getRelativeDamage},
				{"applyDamage", applyDamage},
				{"isAlive", isAlive},
				{"setControlMode", setControlMode},
				{"getFaction", getFaction},
				{"setFaction", setFaction},
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
			//ls_message("state: %d\n", getObject(self)->getState());
			if (getObject(self)->getState() == IActor::ALIVE) {
				return self;
			} else {
				return IONIL(self);
			}
		}
		CREATE_FUNC(IActor)
		
		SET_ENUM(setControlMode, IActor::ControlMode)
		GET_NUMBER(getNumViews)
		GET_NUMBER(getRelativeDamage)
		SET_FLOAT(applyDamage)
		GETTER(Ptr<Faction>, getFaction)
		SETTER(Ptr<Faction>, setFaction)
	};
} // namespace

template<>
void addMapping<IActor>(Ptr<IGame> game, IoState *state) {
	ActorMapping::addMapping(game,state);
}

template<>
IoObject * wrapObject<Ptr<IActor> >(Ptr<IActor> actor, IoState *state) {
	if (!actor) return state->ioNil;
	return ActorMapping::create(actor, state);
}

