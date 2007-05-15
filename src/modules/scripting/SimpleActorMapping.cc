// ---------------------------------------------------------------
// |  SimpleActorMapping                                         |
// ---------------------------------------------------------------


#include <modules/actors/simpleactor.h>
#include <modules/model/model.h>
#include <modules/engines/rigidengine.h>
#include <modules/weaponsys/Armament.h>
#include <modules/weaponsys/Targeter.h>

#include "mappings.h"

struct Engine;
class TargetInfo;

namespace {
	
	struct SimpleActorMapping
        :	public TemplatedObjectMapping<SimpleActor, DynamicCastMapping<SimpleActor> >
	{
		static void addMapping(Ptr<IGame> game, IoState * state) {
			IoObject *lobby = state->lobby;
			
			IoObject *self = proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(lobby, IOSYMBOL("SimpleActor"), self);
		}
		
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
                {"controls", controls},
                {"armament", armament},
                {"targeter", getTargeter},
				//{"setTargetInfo", setTargetInfo},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "SimpleActor"));
			IoObject_setDataPointer_(self, 0);
            IoObject_rawAppendProto_(self, getProtoObject<Ptr<IActor> >(IOSTATE));
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}

		CREATE_FUNC(SimpleActor)
		TAG_FUNC
		
		static IoObject * rawClone(IoObject *self) 
		{ 
			Ptr<IGame> game = unwrapObject<Ptr<IGame> >(
				getProtoObject<Ptr<IGame> >(IOSTATE));
			IoObject *clone = IoObject_rawClonePrimitive(
				IoState_protoWithInitFunction_(IOSTATE, proto));
			retarget(clone, new SimpleActor(game));
			return clone;
		}
		
    	static void free(IoObject * self) {
    		if (IoObject_dataPointer(self)) {
    		    SimpleActor* obj = getObject(self);
    		    obj->rawResetIoObject();
    		    obj->unref();
    		}
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
				getProtoObject<Ptr<IGame> >(IOSTATE));
			Ptr<RigidEngine> engine = new RigidEngine(game);
			engine->construct(M, Ixx, Iyy, Izz);
			engine->applyAngularVelocity(omega);
			
			getObject(self)->setEngine(engine);
			return self;
		}
		
		static IoObject * controls
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("SimpleActor.controls")
            return wrapObject<Ptr<DataNode> >(
                getObject(self)->getControls(),
                IOSTATE);
        }
        
        static IoObject * armament
        (IoObject *self, IoObject *locals, IoMessage *m) {
            BEGIN_FUNC("SimpleActor.armament")
            return wrapObject<Ptr<Armament> >(getObject(self)->getArmament(), IOSTATE);
        }
        
        GETTER(Ptr<Targeter>, getTargeter)

	};
} // namespace

template<>
void addMapping<SimpleActor>(Ptr<IGame> game, IoState *state) {
	SimpleActorMapping::addMapping(game,state);
}

template<>
IoObject * wrapObject(Ptr<SimpleActor> actor, IoState *state) {
	if (!actor) return state->ioNil;
    IoObject *obj = actor->getIoObject();
    return obj?obj:SimpleActorMapping::create(actor,state);
}

template<>
Ptr<SimpleActor> unwrapObject(IoObject * self) {
    return SimpleActorMapping::getObject(self);
}


template<>
IoObject *getProtoObject<Ptr<SimpleActor> >(IoState * state) {
	return IoState_protoWithInitFunction_(state, SimpleActorMapping::proto);
}
