// ---------------------------------------------------------------
// |  ActorStageMapping                                          |
// ---------------------------------------------------------------

#include <vector>
#include <interfaces/IActor.h>
#include <interfaces/IActorStage.h>
#include "mappings.h"

namespace {
	struct ActorStageMapping : public TemplatedObjectMapping<IActorStage> {
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject *lobby = state->lobby;
			
			IoObject *self = proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(lobby,IOSYMBOL("ActorStage"), self);
		}
		
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"addActor", addActor},
				{"removeActor", removeActor},
				{"queryActorsInSphere", queryActorsInSphere},
				{"queryActorsInCylinder", queryActorsInCylinder},
				{"queryActorsInBox", queryActorsInBox},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "ActorStage"));
			IoObject_setDataPointer_(self, 0);
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		static IoObject * queryActorsInSphere
		(IoObject *self, IoObject *locals, IoMessage *m) {
			typedef IActorStage::ActorVector Actors;
			
			BEGIN_FUNC("ActorStage.queryActorsInSphere")
			IOASSERT(IoMessage_argCount(m) == 2, "Expected two arguments")
			Vector midp = unwrapObject<Vector>(IoMessage_locals_valueArgAt_(m, locals, 0));
			float radius = IoMessage_locals_doubleArgAt_(m, locals, 1);
			
			Actors actors;
			getObject(self)->queryActorsInSphere(actors,midp,radius);
			return wrapObject<Actors>(actors, IOSTATE);
		}
		
		static IoObject * queryActorsInCylinder
		(IoObject *self, IoObject *locals, IoMessage *m) {
			typedef IActorStage::ActorVector Actors;
			
			BEGIN_FUNC("ActorStage.queryActorsInCylinder")
			IOASSERT(IoMessage_argCount(m) == 2, "Expected two arguments")
			Vector midp = unwrapObject<Vector>(IoMessage_locals_valueArgAt_(m, locals, 0));
			float radius = IoMessage_locals_doubleArgAt_(m, locals, 1);
			
			Actors actors;
			getObject(self)->queryActorsInCylinder(actors,midp,radius);
			return wrapObject<Actors>(actors, IOSTATE);
		}

		static IoObject * queryActorsInBox
		(IoObject *self, IoObject *locals, IoMessage *m) {
			typedef IActorStage::ActorVector Actors;
			
			BEGIN_FUNC("ActorStage.queryActorsInBox")
			IOASSERT(IoMessage_argCount(m) == 2, "Expected two arguments")
			Vector minv = unwrapObject<Vector>(
				IoMessage_locals_valueArgAt_(m, locals, 0));
			Vector maxv = unwrapObject<Vector>(
				IoMessage_locals_valueArgAt_(m, locals, 1));
			
			Actors actors;
			getObject(self)->queryActorsInBox(actors,minv,maxv);
			return wrapObject<Actors>(actors, IOSTATE);
		}
		
		SETTER(Ptr<IActor>, addActor)
		SETTER(Ptr<IActor>, removeActor)
	};
}

template<>
void addMapping<IActorStage>(Ptr<IGame> game, IoState *state) {
	ActorStageMapping::addMapping(game,state);
}

template<>
IoObject * 
wrapObject(Ptr<IActorStage> config, IoState * state) {
	IoObject *new_object = IOCLONE(
		IoState_protoWithInitFunction_(state, ActorStageMapping::proto));
	ActorStageMapping::retarget(new_object, &*config);
	return new_object;
}

template<>
Ptr<IActorStage> unwrapObject(IoObject * self) {
	return (IActorStage*)IoObject_dataPointer(self);
}

