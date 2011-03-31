// ---------------------------------------------------------------
// |  TankMapping                                               |
// ---------------------------------------------------------------

#include <modules/actors/tank/tank.h>
#include "mappings.h"

template<>
Ptr<Tank> unwrapObject(IoObject * self) {
	return (Tank*)IoObject_dataPointer(self);
}


namespace {
	
    struct TankMapping : public TemplatedObjectMapping<Tank, DynamicCastMapping<Tank> > {
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject *lobby = state->lobby;
			
			IoObject *self = proto(state);
		    IoObject_tag(self)->cloneFunc = (IoTagCloneFunc *)rawClone;
		    
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(lobby, IOSYMBOL("Tank"), self);
		}
		
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"asActor", castfunc<Ptr<Tank>, Ptr<IActor> >},
				{"asSimpleActor", castfunc<Ptr<Tank>, Ptr<SimpleActor> >},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "Tank"));
			IoObject_setDataPointer_(self, 0);
            
            IoObject_rawAppendProto_(self, getProtoObject<Ptr<SimpleActor> >(IOSTATE));
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		static IoObject * create(Ptr<Tank> tank, IoState *state) 
		{
			IoObject *child = IOCLONE(
				IoState_protoWithInitFunction_(state, proto));
			retarget(child, ptr(tank));
			return child;
		}
			
		static IoObject * rawClone(IoObject *self) 
		{ 
			Ptr<IGame> game = unwrapObject<Ptr<IGame> >(
				getProtoObject<Ptr<IGame> >(IOSTATE));
			IoObject *peer = IoObject_rawClonePrimitive(
				IoState_protoWithInitFunction_(IOSTATE, proto));
			Ptr<Tank> clone = new Tank(game, peer);
            retarget(peer, ptr(clone));
            if (getObject(self)) {
                Tank *me = getObject(self);
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
    		    Tank* obj = getObject(self);
    		    obj->rawResetIoObject();
    		    obj->unref();
    		}
    	}

		TAG_FUNC
	};
}

template<>
void addMapping<Tank>(Ptr<IGame> game, IoState *state) {
	TankMapping::addMapping(game,state);
}

template<>
IoObject * 
wrapObject<Ptr<Tank> >(Ptr<Tank> tank, IoState * state) {
	if (!tank) return state->ioNil;
    IoObject *obj = tank->getIoObject();
    return obj?obj:TankMapping::create(tank,state);
}

template<>
IoObject *getProtoObject<Ptr<Tank> >(IoState * state) {
	return IoState_protoWithInitFunction_(state, TankMapping::proto);
}
