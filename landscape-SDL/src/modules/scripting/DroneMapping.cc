// ---------------------------------------------------------------
// |  DroneMapping                                               |
// ---------------------------------------------------------------

#include <modules/actors/drone/drone.h>
#include "mappings.h"

template<>
Ptr<Drone> unwrapObject(IoObject * self) {
	return (Drone*)self->data;
}


namespace {
	
	struct DroneMapping : public TemplatedObjectMapping<Drone> {
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject *lobby = state->lobby;
			
			IoObject *self = proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(lobby, IOSYMBOL("Drone"), self);
		}
		
		TAG_FUNC
		
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"asActor", castfunc<Ptr<Drone>, Ptr<IActor> >},
				{"asSimpleActor", castfunc<Ptr<Drone>, Ptr<SimpleActor> >},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			self->tag = tag(state, "Drone");
			self->data = 0;
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		CREATE_FUNC(Drone)
		
		static IoObject * rawClone(IoObject *self) 
		{ 
			Ptr<IGame> game = unwrapObject<Ptr<IGame> >(
				getProtoObject<IGame>(IOSTATE));
			IoObject *peer = IoObject_rawClonePrimitive(
				IoState_protoWithInitFunction_(IOSTATE, proto));
			retarget(peer, new Drone(game, peer));
			return peer;
		}
	};
}

template<>
void addMapping<Drone>(Ptr<IGame> game, IoState *state) {
	DroneMapping::addMapping(game,state);
}

template<>
IoObject * 
wrapObject(Ptr<Drone> drone, IoState * state) {
	return DroneMapping::create(drone, state);
}


