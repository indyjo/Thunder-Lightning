// ---------------------------------------------------------------
// |  ModelMapping                                               |
// ---------------------------------------------------------------


#include <modules/model/model.h>

#include "mappings.h"

template<>
Ptr<Model> unwrapObject(IoObject * self) {
	return (Model*)IoObject_dataPointer(self);
}

namespace {
	
	struct ModelMapping
	:	public TemplatedObjectMapping<Model>
	{
		static void addMapping(Ptr<IGame> game, IoState * state) {
			IoObject *self = proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(state->lobby, IOSYMBOL("Model"), self);
		}
		
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "Model"));
			IoObject_setDataPointer_(self, 0);
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
		
		CREATE_FUNC(Model)
	};
} // namespace

template<>
void addMapping<Model>(Ptr<IGame> game, IoState *state) {
	ModelMapping::addMapping(game,state);
}

template<>
IoObject * wrapObject(Ptr<Model> model, IoState *state) {
	if (!model) return state->ioNil;
	return ModelMapping::create(model, state);
}

template<>
IoObject *getProtoObject<Ptr<Model> >(IoState * state) {
	return IoState_protoWithInitFunction_(state, ModelMapping::proto);
}
