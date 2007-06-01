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
			    {"setCullmode", setCullmode},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "Model"));
			IoObject_setDataPointer_(self, 0);
			IoObject_setSlot_to_(self, IOSYMBOL("NO_CULLING"),
			    wrapObject((int)JR_CULLMODE_NO_CULLING, IOSTATE));
			IoObject_setSlot_to_(self, IOSYMBOL("CULL_POSITIVE"),
			    wrapObject((int)JR_CULLMODE_CULL_POSITIVE, IOSTATE));
			IoObject_setSlot_to_(self, IOSYMBOL("CULL_NEGATIVE"),
			    wrapObject((int)JR_CULLMODE_CULL_NEGATIVE, IOSTATE));
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
		
		CREATE_FUNC(Model)
		
        static IoObject * setCullmode (
        IoObject *self, IoObject *locals, IoMessage *m) {
        	BEGIN_FUNC("setCullmode")
        	IOASSERT(IoMessage_argCount(m) == 1,
        		"Expected one argument")
        	IoObject *arg = IoMessage_locals_numberArgAt_(
        		m, locals, 0);
        	getObject(self)->setCullmode((jrcullmode_t)IoNumber_asInt(arg));
        	return self;
        }
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
