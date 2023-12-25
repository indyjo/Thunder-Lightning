#ifdef HAVE_IO

// ---------------------------------------------------------------
// |  DroneMapping                                               |
// ---------------------------------------------------------------

#include <modules/actors/drone/drone.h>
#include "mappings.h"


namespace {
	
    struct DroneMapping : public TemplatedObjectMapping<Drone, DynamicCastMapping<Drone> > {
        static const char *const id;
        
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject *lobby = state->lobby;
			
			IoObject *self = proto(state);
			IoState_registerProtoWithId_(state, self, id);
			IoObject_setSlot_to_(lobby, IOSYMBOL("Drone"), self);
		}
		
    	static void free(IoObject * self) {
			//ls_message("Freed %s\n", INFO(self));
    		if (IoObject_dataPointer(self)) {
    		    Drone* obj = getObject(self);
    		    obj->rawResetIoObject();
    		    obj->unref();
    		}
    	}

		TAG_FUNC
		
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"asActor", castfunc<Ptr<Drone>, Ptr<IActor> >},
				{"asSimpleActor", castfunc<Ptr<Drone>, Ptr<SimpleActor> >},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "Drone"));
			IoObject_setDataPointer_(self, 0);
            IoObject_rawAppendProto_(self, getProtoObject<Ptr<SimpleActor> >(IOSTATE));
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		CREATE_FUNC(Drone, id)
		
		static IoObject * rawClone(IoObject *self) 
		{ 
			Ptr<IGame> game = unwrapObject<Ptr<IGame> >(
				getProtoObject<Ptr<IGame> >(IOSTATE));
			IoObject *peer = IoObject_rawClonePrimitive(self);
            Drone *clone = new Drone(game, peer);
			retarget(peer, clone);
            clone->init();
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
    
    const char *const DroneMapping::id = "Drone";
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
	return IoState_protoWithId_(state, DroneMapping::id);
}

#endif
