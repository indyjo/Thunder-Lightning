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
			IoObject_setSlot_to_(lobby, IOSTRING("Drone"), self);
		}
		
		static IoTag *tag(void * state, char * name) {
		    IoTag *tag = IoTag_newWithName_(name);
		    tag->state = state;
		    tag->cloneFunc = (TagCloneFunc *)rawClone;
		    tag->markFunc  = (TagMarkFunc *)mark;
		    tag->freeFunc  = (TagFreeFunc *)free;
		    //tag->writeToStoreFunc  = (TagWriteToStoreFunc *)IoFile_writeToStore_;
		    //tag->readFromStoreFunc = (TagReadFromStoreFunc *)IoFile_readFromStore_;
		    return tag;
		}
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"asActor", castfunc<Ptr<Drone>, Ptr<IActor> >},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			self->tag = tag(state, "Drone");
			self->data = 0;
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		static IoObject * create(Ptr<Drone> faction, IoState *state) 
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
wrapObject<Ptr<Drone> >(Ptr<Drone> faction, IoState * state) {
	return DroneMapping::create(faction, state);
}


