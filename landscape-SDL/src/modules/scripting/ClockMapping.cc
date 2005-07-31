// ---------------------------------------------------------------
// |  ClockMapping                                               |
// ---------------------------------------------------------------


#include <modules/clock/clock.h>
#include "mappings.h"

namespace {
	
	struct ClockMapping : public TemplatedObjectMapping<Clock> {
		static void addMapping(Ptr<IGame> game, IoState * state) {
			IoObject * self = proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(state->lobby, IOSYMBOL("Clock"), self);
			retarget(self, ptr(game->getClock()));
		}
	
		static IoObject *proto(void *state) {
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
			IoObject_addMethodTable_(self, methodTable);
			return self;
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

