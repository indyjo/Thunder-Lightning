// ---------------------------------------------------------------
// |  ProviderMapping                                            |
// ---------------------------------------------------------------

#include <interfaces/IPositionProvider.h>
#include <interfaces/IMovementProvider.h>
#include <modules/math/SpecialMatrices.h>
#include "mappings.h"

namespace {
	
	struct PositionProviderMapping
        :	public TemplatedObjectMapping<IPositionProvider, DynamicCastMapping<IPositionProvider> >
	{
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject * self =  proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(state->lobby, IOSYMBOL("PositionProvider"), self);
			self->data.ptr = 0;
		}
		static IoObject *proto(void *state) {
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
			self->data.ptr = 0;
			
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
		CREATE_FUNC(IPositionProvider)
		
		GET_VECTOR(getLocation)
		GET_VECTOR(getUpVector)
		GET_VECTOR(getFrontVector)
		GET_VECTOR(getRightVector)
	};
	
	struct MovementProviderMapping
	:	public TemplatedObjectMapping<IMovementProvider, DynamicCastMapping<IMovementProvider> >
	{
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject * self =  proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(state->lobby, IOSYMBOL("MovementProvider"), self);
			self->data.ptr = 0;
		}
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"getMovementVector", getMovementVector},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			self->tag = tag(state, "MovementProvider");
			self->data.ptr = 0;
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
		CREATE_FUNC(IMovementProvider)
		
		GET_VECTOR(getMovementVector)
	};
}

template<>
void addMapping<IPositionProvider>(Ptr<IGame> game, IoState *state) {
	PositionProviderMapping::addMapping(game,state);
}

template<>
IoObject * wrapObject<Ptr<IPositionProvider> >
(Ptr<IPositionProvider> pp, IoState *state) {
	return PositionProviderMapping::create(pp, state);
}

template<>
Ptr<IPositionProvider> unwrapObject<Ptr<IPositionProvider> >(IoObject * self) {
    return PositionProviderMapping::getObject(self);
}


template<>
IoObject *getProtoObject<Ptr<IPositionProvider> >(IoState * state) {
	return IoState_protoWithInitFunction_(state, PositionProviderMapping::proto);
}


template<>
void addMapping<IMovementProvider>(Ptr<IGame> game, IoState *state) {
	MovementProviderMapping::addMapping(game,state);
}


template<>
IoObject * wrapObject
(Ptr<IMovementProvider> pp, IoState *state) {
	return MovementProviderMapping::create(pp, state);
}

template<>
Ptr<IMovementProvider> unwrapObject(IoObject * self) {
	return MovementProviderMapping::getObject(self);
}

template<>
IoObject *getProtoObject<Ptr<IMovementProvider> >(IoState * state) {
	return IoState_protoWithInitFunction_(state, MovementProviderMapping::proto);
}
