// ---------------------------------------------------------------
// |  DroneMapping                                               |
// ---------------------------------------------------------------

#include <modules/actors/drone/drone.h>
#include "mappings.h"


namespace {
	
    struct DroneMapping : public TemplatedObjectMapping<Drone, DynamicCastMapping<Drone> > {
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
			self->data.ptr = 0;
            IoObject_rawAppendProto_(self, getProtoObject<Ptr<SimpleActor> >(IOSTATE));
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		CREATE_FUNC(Drone)
		
		static IoObject * rawClone(IoObject *self) 
		{ 
			Ptr<IGame> game = unwrapObject<Ptr<IGame> >(
				getProtoObject<Ptr<IGame> >(IOSTATE));
			IoObject *peer = IoObject_rawClonePrimitive(
				IoState_protoWithInitFunction_(IOSTATE, proto));
            Drone *clone = new Drone(game, peer);
			retarget(peer, clone);
            if (getObject(self)) {
                Drone *me = getObject(self);
                clone->setFaction(me->getFaction());
                clone->setLocation(me->getLocation());
                clone->setMovementVector(me->getMovementVector());
                clone->setOrientation(me->getUpVector(),
                    me->getRightVector(),
                    me->getFrontVector());
            }
			return peer;
		}
	};
}

template<>
void addMapping<Drone>(Ptr<IGame> game, IoState *state) {
	DroneMapping::addMapping(game,state);
}

template<>
Ptr<Drone> unwrapObject(IoObject * self) {
    return DroneMapping::getObject(self);
}

template<>
IoObject * 
wrapObject(Ptr<Drone> drone, IoState * state) {
	if (!drone) return state->ioNil;
    IoObject *obj = drone->getIoObject();
    return obj?obj:DroneMapping::create(drone,state);
}

template<>
IoObject *getProtoObject<Ptr<Drone> >(IoState * state) {
	return IoState_protoWithInitFunction_(state, DroneMapping::proto);
}
