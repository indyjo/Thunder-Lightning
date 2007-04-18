// ---------------------------------------------------------------
// |  ReceiverMapping                                            |
// ---------------------------------------------------------------

#include <interfaces/IPositionReceiver.h>
#include <interfaces/IMovementReceiver.h>
#include <modules/math/SpecialMatrices.h>
#include "mappings.h"

namespace {
	
	struct PositionReceiverMapping
        :	public TemplatedObjectMapping<IPositionReceiver, DynamicCastMapping<IPositionReceiver> >
	{
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject * self =  proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(state->lobby, IOSYMBOL("PositionReceiver"), self);
		}
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"setLocation", setLocation},
				{"setOrientation", setOrientation},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "PositionReceiver"));
			IoObject_setDataPointer_(self, 0);
			
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
        :	public TemplatedObjectMapping<IMovementReceiver, DynamicCastMapping<IMovementReceiver> >
	{
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject * self =  proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(state->lobby, IOSYMBOL("MovementReceiver"), self);
		}
		
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"setMovementVector", setMovementVector},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "MovementReceiver"));
			IoObject_setDataPointer_(self, 0);
			
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
    return PositionReceiverMapping::getObject(self);
}

template<>
IoObject *getProtoObject<Ptr<IPositionReceiver> >(IoState * state) {
	return IoState_protoWithInitFunction_(state, PositionReceiverMapping::proto);
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
    return MovementReceiverMapping::getObject(self);
}

template<>
IoObject *getProtoObject<Ptr<IMovementReceiver> >(IoState * state) {
	return IoState_protoWithInitFunction_(state, MovementReceiverMapping::proto);
}
