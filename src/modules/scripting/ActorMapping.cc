// ---------------------------------------------------------------
// |  ActorMapping                                               |
// ---------------------------------------------------------------


#include <interfaces/IActor.h>
#include <interfaces/IProjectile.h>
#include <Faction.h>

#include "mappings.h"

namespace {
	
	struct ActorMapping
        :	public TemplatedObjectMapping<IActor, DynamicCastMapping<IActor> >
	{
		static void addMapping(Ptr<IGame> game, IoState * state) {
			IoObject *lobby = state->lobby;
			
			IoObject *self = proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(lobby, IOSYMBOL("Actor"), self);
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
				{"hasControlMode", hasControlMode},
                {"controlMode", getControlMode},
				{"getFaction", getFaction},
				{"setFaction", setFaction},
                {"passMessage", passMessage},
                {"kill", kill},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "Actor"));
			IoObject_setDataPointer_(self, 0);
            IoObject_rawAppendProto_(self, getProtoObject<Ptr<IPositionProvider> >(IOSTATE));
            IoObject_rawAppendProto_(self, getProtoObject<Ptr<IPositionReceiver> >(IOSTATE));
            IoObject_rawAppendProto_(self, getProtoObject<Ptr<IMovementProvider> >(IOSTATE));
            IoObject_rawAppendProto_(self, getProtoObject<Ptr<IMovementReceiver> >(IOSTATE));

			IoObject_addTaglessMethodTable_(self, methodTable);
			return self;
		}
		
		static IoObject *isAlive(IoObject *self, IoObject*locals, IoObject*m) {
			BEGIN_FUNC("Actor.isAlive")
			//ls_message("state: %d\n", getObject(self)->getState());
			if (getObject(self)->getState() == IActor::ALIVE) {
				return IOTRUE(self);
			} else {
				return IOFALSE(self);
			}
		}
		CREATE_FUNC(IActor)
		
		SET_ENUM(setControlMode, IActor::ControlMode)
		GET_NUMBER(getControlMode)

        static IoObject * hasControlMode
        (IoObject *self, IoObject *locals, IoMessage *m) {
            BEGIN_FUNC("SimpleActor.hasControlMode")
            IOASSERT(IoMessage_argCount(m) == 1, "Expected one argument")
            IActor::ControlMode arg =
                (IActor::ControlMode) IoMessage_locals_intArgAt_(m, locals, 0);
            return wrapObject(getObject(self)->hasControlMode(arg), IOSTATE);
        }
		
		GET_NUMBER(getNumViews)
		GET_NUMBER(getRelativeDamage)
		SET_FLOAT(applyDamage)
		GETTER(Ptr<Faction>, getFaction)
		SETTER(Ptr<Faction>, setFaction)
		
        static IoObject *passMessage(IoObject *self, IoObject*locals, IoObject*m) {
			BEGIN_FUNC("Actor.message")
            IOASSERT(IoMessage_argCount(m) == 2, "Expected two arguments")
			//ls_message("state: %d\n", getObject(self)->getState());
            const char * name = IoMessage_locals_cStringArgAt_(m, locals, 0);
            IoObject * args = IoMessage_locals_valueArgAt_(m, locals, 1);

            getObject(self)->message(name, args);
			return IONIL(self);
		}
		
		VOID_FUNC(kill)
	};
} // namespace

template<>
void addMapping<IActor>(Ptr<IGame> game, IoState *state) {
	ActorMapping::addMapping(game,state);
}


template<>
Ptr<IActor> unwrapObject<Ptr<IActor> >(IoObject * self) {
    return ActorMapping::getObject(self);
}

template<>
IoObject * wrapObject<Ptr<IActor> >(Ptr<IActor> actor, IoState *state) {
    if (!actor) return state->ioNil;
    IoObject *obj = actor->getIoObject();
    if (!obj) obj = ActorMapping::create(actor,state);
    IoState_stackRetain_(state, obj);
    return obj;
}

template<>
IoObject *getProtoObject<Ptr<IActor> >(IoState * state) {
	return IoState_protoWithInitFunction_(state, ActorMapping::proto);
}
