// ---------------------------------------------------------------
// |  ProviderMapping                                            |
// ---------------------------------------------------------------

#include <interfaces/IPositionProvider.h>
#include <modules/math/SpecialMatrices.h>
#include "mappings.h"

namespace {
	
	struct PositionProviderMapping
	:	public TemplatedObjectMapping<IPositionProvider>
	{
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject * self =  proto(state);
			IoObject_setSlot_to_(state->lobby, IOSTRING("PositionProvider"), self);
			self->data = 0;
		}
		static IoObject *proto(IoState *state) {
			IoMethodTable methodTable[] = {
				/* standard I/O */
				{"getLocation", getLocation},
				{"getUpVector", getUpVector},
				{"getRightVector", getRightVector},
				{"getFrontVector", getFrontVector},
				{"getOrientation", getOrientation},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			self->tag = tag(state, "PositionProvider");
			
			self->data = 0;
			IoState_registerProtoWithFunc_(state, self,
				(IoStateProtoFunc*) proto);
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
		
		static IoObject * getOrientation
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("PositionProvider.getOrientation")
			Vector up,right,front;
			getObject(self)->getOrientation(&up,&right,&front);
			return wrapObject<Matrix3>(
				MatrixFromColumns(right,up,front),
				(IoState*) self->tag->state );
		}
		
		GET_VECTOR(getLocation)
		GET_VECTOR(getUpVector)
		GET_VECTOR(getFrontVector)
		GET_VECTOR(getRightVector)
	};
}

template<>
void addMapping<IPositionProvider>(Ptr<IGame> game, IoState *state) {
	PositionProviderMapping::addMapping(game,state);
}

template<>
IoObject * wrapObject<Ptr<IPositionProvider> >
(Ptr<IPositionProvider> pp, IoState *state) {
	IoObject *new_object = PositionProviderMapping::rawClone(
		IoObject_getSlot_(
			state->lobby,
			IoState_stringWithCString_(state, "PositionProvider")));
	new_object->data = &*pp;
	return new_object;
}

template<>
Ptr<IPositionProvider> unwrapObject<Ptr<IPositionProvider> >(IoObject * self) {
	return (IPositionProvider*) self->data;
}

