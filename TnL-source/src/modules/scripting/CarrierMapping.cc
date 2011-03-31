// ---------------------------------------------------------------
// |  CarrierMapping                                             |
// ---------------------------------------------------------------

#include <modules/actors/carrier/Carrier.h>
#include "mappings.h"

template<>
Ptr<Carrier> unwrapObject(IoObject * self) {
	return (Carrier*)IoObject_dataPointer(self);
}


namespace {
	
    struct CarrierMapping : public TemplatedObjectMapping<Carrier, DynamicCastMapping<Carrier> > {
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject *lobby = state->lobby;
			
			IoObject *self = proto(state);
		    IoObject_tag(self)->cloneFunc = (IoTagCloneFunc *)rawClone;
		    
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(lobby, IOSYMBOL("Carrier"), self);
		}
		
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"asActor", castfunc<Ptr<Carrier>, Ptr<IActor> >},
				{"asSimpleActor", castfunc<Ptr<Carrier>, Ptr<SimpleActor> >},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "Carrier"));
			IoObject_setDataPointer_(self, 0);
            
            IoObject_rawAppendProto_(self, getProtoObject<Ptr<SimpleActor> >(IOSTATE));
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		static IoObject * create(Ptr<Carrier> carrier, IoState *state) 
		{
			IoObject *child = IOCLONE(
				IoState_protoWithInitFunction_(state, proto));
			retarget(child, ptr(carrier));
			return child;
		}
			
		static IoObject * rawClone(IoObject *self) 
		{ 
			Ptr<IGame> game = unwrapObject<Ptr<IGame> >(
				getProtoObject<Ptr<IGame> >(IOSTATE));
			IoObject *peer = IoObject_rawClonePrimitive(
				IoState_protoWithInitFunction_(IOSTATE, proto));
			Ptr<Carrier> clone = new Carrier(game, peer);
            retarget(peer, ptr(clone));
            if (getObject(self)) {
                Carrier *me = getObject(self);
                clone->setFaction(me->getFaction());
                clone->setLocation(me->getLocation());
                clone->setMovementVector(me->getMovementVector());
                clone->setOrientation(me->getUpVector(),
                    me->getRightVector(),
                    me->getFrontVector());
            }
			return peer;
		}
		
    	static void free(IoObject * self) {
    		if (IoObject_dataPointer(self)) {
    		    Carrier* obj = getObject(self);
    		    obj->rawResetIoObject();
    		    obj->unref();
    		}
    	}

		TAG_FUNC
	};
}

template<>
void addMapping<Carrier>(Ptr<IGame> game, IoState *state) {
	CarrierMapping::addMapping(game,state);
}

template<>
IoObject * 
wrapObject<Ptr<Carrier> >(Ptr<Carrier> carrier, IoState * state) {
	if (!carrier) return state->ioNil;
    IoObject *obj = carrier->getIoObject();
    return obj?obj:CarrierMapping::create(carrier,state);
}

template<>
IoObject *getProtoObject<Ptr<Carrier> >(IoState * state) {
	return IoState_protoWithInitFunction_(state, CarrierMapping::proto);
}
