// ---------------------------------------------------------------
// |  EnvironmentMapping                                              |
// ---------------------------------------------------------------

#include <modules/environment/environment.h>
#include "mappings.h"

namespace {
	
	struct EnvironmentMapping : public TemplatedObjectMapping<Environment> {
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject *lobby = state->lobby;
			
			IoObject *self = proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(lobby, IOSYMBOL("Environment"), self);
			retarget(self, &*thegame->getEnvironment());
		}
		
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"getFogColor", getFogColor},
				{"setFogColor", setFogColor},
				{"getGroundFogMin", getGroundFogMin},
				{"setGroundFogMin", setGroundFogMin},
				{"getGroundFogMax", getGroundFogMax},
				{"setGroundFogMax", setGroundFogMax},
				{"getGroundFogRange", getGroundFogRange},
				{"setGroundFogRange", setGroundFogRange},
				{"getClipMin", getClipMin},
				{"setClipMin", setClipMin},
				{"getClipMax", getClipMax},
				{"setClipMax", setClipMax},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "Environment"));
			IoObject_setDataPointer_(self, 0);
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
		
		GET_VECTOR(getFogColor)
		SET_VECTOR(setFogColor)
		GET_NUMBER(getGroundFogMin)
		SET_FLOAT(setGroundFogMin)
		GET_NUMBER(getGroundFogMax)
		SET_FLOAT(setGroundFogMax)
		GET_NUMBER(getGroundFogRange)
		SET_FLOAT(setGroundFogRange)
		GET_NUMBER(getClipMin)
		SET_FLOAT(setClipMin)
		GET_NUMBER(getClipMax)
		SET_FLOAT(setClipMax)
	
	};
}

template<>
void addMapping<Environment>(Ptr<IGame> game, IoState *state) {
	EnvironmentMapping::addMapping(game,state);
}

template<>
IoObject * 
wrapObject(Ptr<Environment> environment, IoState * state) {
	IoObject *new_object = IOCLONE(
		IoState_protoWithInitFunction_(state, EnvironmentMapping::proto));
	EnvironmentMapping::retarget(new_object, ptr(environment));
	return new_object;
}

template<>
Ptr<Environment> unwrapObject(IoObject * self) {
	return (Environment*) IoObject_dataPointer(self);
}

