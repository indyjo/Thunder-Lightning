// ---------------------------------------------------------------
// |  ModelManMapping                                            |
// ---------------------------------------------------------------


#include <modules/model/modelman.h>
#include <modules/model/model.h>

#include "mappings.h"

namespace {
	
	struct ModelManMapping : public TemplatedObjectMapping<IModelMan> {
		static void addMapping(Ptr<IGame> game, IoState * state) {
			IoObject * self = proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(state->lobby, IOSYMBOL("ModelMan"), self);
			retarget(self, ptr(game->getModelMan()));
		}
	
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"query", query},
				{"flush", flush},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "ModelMan"));
			IoObject_setDataPointer_(self, 0);
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
		
		VOID_FUNC(flush)
		
		static IoObject * query
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("ModelMan.query")
			IOASSERT(IoMessage_argCount(m) == 1,"Expected one argument")
			const char * name = 
				IoMessage_locals_cStringArgAt_(m, locals, 0);
			return wrapObject<Ptr<Model> >(
				getObject(self)->query(name), IOSTATE);
		}
	};
}

template<>
void addMapping<IModelMan>(Ptr<IGame> game, IoState *state) {
	ModelManMapping::addMapping(game,state);
}

