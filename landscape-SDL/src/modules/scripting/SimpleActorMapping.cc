// ---------------------------------------------------------------
// |  SimpleActorMapping                                         |
// ---------------------------------------------------------------


#include <modules/actors/simpleactor.h>
#include <modules/model/model.h>
#include <modules/engines/rigidengine.h>

#include "mappings.h"

struct Engine;
struct TargetInfo;

template<>
Ptr<SimpleActor> unwrapObject(IoObject * self) {
	return (SimpleActor*)self->data;
}

namespace {
	
	struct SimpleActorMapping
	:	public TemplatedObjectMapping<SimpleActor>
	{
		static void addMapping(Ptr<IGame> game, IoState * state) {
			IoObject *lobby = state->lobby;
			
			IoObject *self = proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(lobby, IOSTRING("SimpleActor"), self);
		}
		
		TAG_FUNC
		
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"asActor",
					castfunc<Ptr<SimpleActor>, Ptr<IActor> >},
				{"asPositionProvider",
					castfunc<Ptr<SimpleActor>, Ptr<IPositionProvider> >},
				{"asMovementProvider",
					castfunc<Ptr<SimpleActor>, Ptr<IMovementProvider> >},
				{"asPositionReceiver",
					castfunc<Ptr<SimpleActor>, Ptr<IPositionReceiver> >},
				{"asMovementReceiver",
					castfunc<Ptr<SimpleActor>, Ptr<IMovementReceiver> >},
				//{"getEngine", getEngine},
				//{"setEngine", setEngine},
				{"setModel", setModel},
				{"makeRigid", makeRigid},
				//{"setTargetInfo", setTargetInfo},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			self->tag = tag(state, "SimpleActor");
			self->data = 0;
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}

		CREATE_FUNC(SimpleActor)
		
		static IoObject * rawClone(IoObject *self) 
		{ 
			Ptr<IGame> game = unwrapObject<Ptr<IGame> >(
				getProtoObject<IGame>(IOSTATE));
			IoObject *clone = IoObject_rawClonePrimitive(
				IoState_protoWithInitFunction_(IOSTATE, proto));
			retarget(clone, new SimpleActor(game));
			return clone;
		}
		
		SETTER(Ptr<Model>, setModel)
		//GETTER(Ptr<Engine>, getEngine)
		//SETTER(Ptr<Engine>, setEngine)
		
		static IoObject * makeRigid
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("SimpleActor.makeRigid")
			IOASSERT(IoMessage_argCount(m) == 5,"Expected 5 arguments")
			
			float M   = IoMessage_locals_doubleArgAt_(m, locals, 0);
			float Ixx = IoMessage_locals_doubleArgAt_(m, locals, 1);
			float Iyy = IoMessage_locals_doubleArgAt_(m, locals, 2);
			float Izz = IoMessage_locals_doubleArgAt_(m, locals, 3);
			Vector omega = unwrapObject<Vector>(
				IoMessage_locals_valueArgAt_(m, locals, 4));
			Ptr<IGame> game = unwrapObject<Ptr<IGame> >(
				getProtoObject<IGame>(IOSTATE));
			Ptr<RigidEngine> engine = new RigidEngine(game);
			engine->construct(M, Ixx, Iyy, Izz);
			engine->applyAngularVelocity(omega);
			
			getObject(self)->setEngine(engine);
			return self;
		}
		
	};
} // namespace

template<>
void addMapping<SimpleActor>(Ptr<IGame> game, IoState *state) {
	SimpleActorMapping::addMapping(game,state);
}

template<>
IoObject * wrapObject(Ptr<SimpleActor> actor, IoState *state) {
	if (!actor) return state->ioNil;
	return SimpleActorMapping::create(actor, state);
}

