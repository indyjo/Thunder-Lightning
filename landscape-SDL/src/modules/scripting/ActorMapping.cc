// ---------------------------------------------------------------
// |  ActorMapping                                               |
// ---------------------------------------------------------------


#include <interfaces/IActor.h>

#include "mappings.h"

namespace {
	
	struct ActorMapping {
		typedef IoObject IoActor;
		
		static IoActor * actor;
		static IoActor * AUTOMATIC;
		static IoActor * MANUAL;
		static IoActor * UNCONTROLLED;
	
		static IActor* getObject(IoActor *a) { return ((IActor*)a->data); }
		
		static void addMapping(Ptr<IGame> game, IoState * state) {
			IoObject *object = state->mainActor;
			IoObject *lobby = state->lobby;
			
			actor = proto(state);
			IoObject_setSlot_to_(lobby,
				IoState_stringWithCString_(state, "Actor"), actor);
			
			AUTOMATIC    = IoObject_rawClone(object);
			MANUAL       = IoObject_rawClone(object);
			UNCONTROLLED = IoObject_rawClone(object);
			
			IoObject_setSlot_to_(actor,
				IoState_stringWithCString_(state, "AUTOMATIC"), AUTOMATIC);
			IoObject_setSlot_to_(actor,
				IoState_stringWithCString_(state, "MANUAL"), MANUAL);
			IoObject_setSlot_to_(actor,
				IoState_stringWithCString_(state, "UNCONTROLLED"), UNCONTROLLED);
			
		}
		
		static IoTag *tag(IoState * state, char * name) {
		    IoTag *tag = IoTag_newWithName_(name);
		    tag->state = state;
		    tag->cloneFunc = (TagCloneFunc *)rawClone;
		    tag->markFunc  = (TagMarkFunc *)mark;
		    tag->freeFunc  = (TagFreeFunc *)free;
		    //tag->writeToStoreFunc  = (TagWriteToStoreFunc *)IoFile_writeToStore_;
		    //tag->readFromStoreFunc = (TagReadFromStoreFunc *)IoFile_readFromStore_;
		    return tag;
		}
	
		static IoActor *proto(IoState *state) {
			IoMethodTable methodTable[] = {
				/* standard I/O */
				{"isAlive", isAlive},
				{"setControlMode", setControlMode},
				{"getLocation", getLocation},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			self->tag = tag(state, "Actor");
			
			self->data = 0;
			IoState_registerProtoWithFunc_(state, self,
				(IoStateProtoFunc*) proto);
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		static void mark(IoActor * self) {
			if (self->data) getObject(self)->ref();
		}
		static void free(IoActor * self) {
			if (self->data) getObject(self)->unref();
		}
		
		static IoActor *rawClone(IoActor *self) 
		{ 
			IoObject *child = IoObject_rawClonePrimitive(self);
			child->data = self->data;
			return child;
		}
		
		static IoActor *createWrapped(Ptr<IActor> p) {
			IoObject *child = IoObject_rawClonePrimitive(actor);
			child->data = &*p;
			return child;
		}
		
		static IoObject *isAlive(IoActor *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("Actor.isAlive")
			if (getObject(self)->getState()==IActor::ALIVE) {
				return self;
			} else {
				return IONIL(self);
			}
		}
		
		static IoObject *setControlMode(IoActor *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("Actor.setControlMode")
			IOASSERT(IoMessage_argCount(m) == 1,"Expected one argument")
			IoObject *arg = IoMessage_locals_objectArgAt_(m, locals, 0);
			IOASSERT(arg == AUTOMATIC || arg == MANUAL || arg == UNCONTROLLED,
				"Argument must be one of Actor AUTOMATIC, Actor MANUAL or Actor UNCONTROLLED")
			if (arg == AUTOMATIC) {
				getObject(self)->setControlMode(IActor::AUTOMATIC);
			} else if ( arg == MANUAL ) {
				getObject(self)->setControlMode(IActor::MANUAL);
			} else {
				getObject(self)->setControlMode(IActor::UNCONTROLLED);
			}
			return self;
		}
		
		static IoObject *getLocation
		(IoActor *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("Actor.getLocation")
			return wrapObject<Vector>(getObject(self)->getLocation(),
				IOSTATE);
		}
	};
	ActorMapping::IoActor * ActorMapping::actor;
	ActorMapping::IoActor * ActorMapping::AUTOMATIC;
	ActorMapping::IoActor * ActorMapping::MANUAL;
	ActorMapping::IoActor * ActorMapping::UNCONTROLLED;
}

template<>
void addMapping<IActor>(Ptr<IGame> game, IoState *state) {
	ActorMapping::addMapping(game,state);
}

template<>
IoObject * wrapObject<Ptr<IActor> >(Ptr<IActor> p, IoState *state) {
	return ActorMapping::createWrapped(p);
}

template<>
Ptr<IActor> unwrapObject<Ptr<IActor> >(IoObject * self) {
	return (IActor*)self->data;
}

