// ---------------------------------------------------------------
// |  TankMapping                                               |
// ---------------------------------------------------------------

#include <modules/actors/tank/tank.h>
#include "mappings.h"

template<>
Ptr<Tank> unwrapObject(IoObject * self) {
	return (Tank*)self->data;
}


namespace {
	
	struct TankMapping : public TemplatedObjectMapping<Tank> {
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject *lobby = state->lobby;
			
			IoObject *self = proto(state);
		    self->tag->cloneFunc = (TagCloneFunc *)rawClone;
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(lobby, IOSTRING("Tank"), self);
		}
		
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"asActor", castfunc<Ptr<Tank>, Ptr<IActor> >},
				{"asSimpleActor", castfunc<Ptr<Tank>, Ptr<SimpleActor> >},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			self->tag = tag(state, "Tank");
			self->data = 0;
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		static IoObject * create(Ptr<Tank> faction, IoState *state) 
		{
			IoObject *child = IoObject_rawClonePrimitive(
				IoState_protoWithInitFunction_(state, proto));
			retarget(child, ptr(faction));
			return child;
		}
			
		static IoObject * rawClone(IoObject *self) 
		{ 
			Ptr<IGame> game = unwrapObject<Ptr<IGame> >(
				getProtoObject<IGame>(IOSTATE));
			return create(new Tank(game), IOSTATE);
		}
	};
}

template<>
void addMapping<Tank>(Ptr<IGame> game, IoState *state) {
	TankMapping::addMapping(game,state);
}

template<>
IoObject * 
wrapObject<Ptr<Tank> >(Ptr<Tank> tank, IoState * state) {
	return TankMapping::create(tank, state);
}


