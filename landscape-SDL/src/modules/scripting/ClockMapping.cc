// ---------------------------------------------------------------
// |  ClockMapping                                               |
// ---------------------------------------------------------------


#include <modules/clock/clock.h>
#include "mappings.h"

namespace {
	
	struct ClockMapping {
		static Clock* getObject(IoObject *a) { return ((Clock*)a->data); }
		
		static void addMapping(Ptr<IGame> game, IoState * state) {
			IoObject *lobby = state->lobby;
			IoObject * clock = proto(state);
			IoObject_setSlot_to_(lobby,
				IoState_stringWithCString_(state, "Clock"), clock);
			clock->data = &*game->getClock();
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
	
		static IoObject *proto(IoState *state) {
			IoMethodTable methodTable[] = {
				{"getFrameDelta", getFrameDelta},
				{"getStepDelta", getStepDelta},
				{"getRealFrameDelta", getRealFrameDelta},
				{"getRealStepDelta", getRealStepDelta},
				{"getTimeFactor", getTimeFactor},
				{"setTimeFactor", setTimeFactor},
				{"pause", pause},
				{"resume", resume},
				{"isPaused", isPaused},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			self->tag = tag(state, "Clock");
			
			self->data = 0;
			IoState_registerProtoWithFunc_(state, self,
				(IoStateProtoFunc*) proto);
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		static void mark(IoObject * self) {
			if (self->data) getObject(self)->ref();
		}
		static void free(IoObject * self) {
			if (self->data) getObject(self)->unref();
		}
		
		static IoObject *rawClone(IoObject *self) 
		{ 
			IoObject *child = IoObject_rawClonePrimitive(self);
			child->data = self->data;
			return child;
		}
		
		GET_NUMBER(getFrameDelta)
		GET_NUMBER(getRealFrameDelta)
		GET_NUMBER(getStepDelta)
		GET_NUMBER(getRealStepDelta)
		GET_NUMBER(getTimeFactor)
		SET_FLOAT(setTimeFactor)
		VOID_FUNC(pause)
		VOID_FUNC(resume)
		GET_BOOLEAN(isPaused)
	};
}

template<>
void addMapping<Clock>(Ptr<IGame> game, IoState *state) {
	ClockMapping::addMapping(game,state);
}

template<>
Ptr<Clock> unwrapObject<Ptr<Clock> >(IoObject * self) {
	return (Clock*)self->data;
}

