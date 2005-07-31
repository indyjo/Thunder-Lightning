// ---------------------------------------------------------------
// |  ReceiverMapping                                            |
// ---------------------------------------------------------------

#include <interfaces/IPositionReceiver.h>
#include <interfaces/IMovementReceiver.h>
#include <modules/math/SpecialMatrices.h>
#include "mappings.h"

namespace {
	
	struct PositionReceiverMapping
	:	public TemplatedObjectMapping<IPositionReceiver>
	{
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject * self =  proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(state->lobby, IOSYMBOL("PositionReceiver"), self);
			self->data = 0;
		}
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"setLocation", setLocation},
				{"setOrientation", setOrientation},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			self->tag = tag(state, "PositionReceiver");
			self->data = 0;
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
			
		static IoObject * setOrientation
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("PositionReceiver.setOrientation")
			IOASSERT(IoMessage_argCount(m) == 1, "Expected one argument")
			IoObject *arg = IoMessage_locals_valueArgAt_(m, locals, 0);								\
			Matrix3 orient = unwrapObject<Matrix3>(arg);
			getObject(self)->setOrientation(
				orient.column(1), // up
				orient.column(0), // right
				orient.column(2));// front
			return self;
		}
		
		CREATE_FUNC(IPositionReceiver)
		
		SET_VECTOR(setLocation)
	};
	
	struct MovementReceiverMapping
	:	public TemplatedObjectMapping<IMovementReceiver>
	{
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject * self =  proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(state->lobby, IOSYMBOL("MovementReceiver"), self);
			self->data = 0;
		}
		
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"setMovementVector", setMovementVector},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			self->tag = tag(state, "MovementReceiver");
			self->data = 0;
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
		
		CREATE_FUNC(IMovementReceiver)
		
		SET_VECTOR(setMovementVector)
	};
}

template<>
void addMapping<IPositionReceiver>(Ptr<IGame> game, IoState *state) {
	PositionReceiverMapping::addMapping(game,state);
}

template<>
IoObject * wrapObject<Ptr<IPositionReceiver> >
(Ptr<IPositionReceiver> pp, IoState *state) {
	return PositionReceiverMapping::create(pp, state);
}

template<>
Ptr<IPositionReceiver> unwrapObject<Ptr<IPositionReceiver> >(IoObject * self) {
	return (IPositionReceiver*) self->data;
}


template<>
void addMapping<IMovementReceiver>(Ptr<IGame> game, IoState *state) {
	MovementReceiverMapping::addMapping(game,state);
}

template<>
IoObject * wrapObject
(Ptr<IMovementReceiver> pp, IoState *state) {
	return MovementReceiverMapping::create(pp, state);
}

template<>
Ptr<IMovementReceiver> unwrapObject(IoObject * self) {
	return (IMovementReceiver*) self->data;
}
