// ---------------------------------------------------------------
// |  GameMapping                                                |
// ---------------------------------------------------------------

#include <interfaces/IActor.h>
#include "mappings.h"

namespace {
	
	struct GameMapping : public TemplatedObjectMapping<IGame> {
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject *object = state->mainActor;
			IoObject *lobby = state->lobby;
			
			IoObject *game = proto(state);
			IoObject_setSlot_to_(lobby,
				IoState_stringWithCString_(state, "Game"), game);
			game->data = &*thegame;
			getObject(game)->ref();
		}
		
		static IoObject *proto(IoState *state) {
			IoMethodTable methodTable[] = {
				/* standard I/O */
				{"getCurrentActor", getCurrentActor},
				{"setCurrentActor", setCurrentActor},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			self->tag = tag(state, "Game");
			
			self->data = 0;
			IoState_registerProtoWithFunc_(state, self,
				(IoStateProtoFunc*) proto);
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		static IoObject *
		getCurrentActor(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("Game.getCurrentActor")
			return wrapObject<Ptr<IActor> >(
				getObject(self)->getCurrentlyControlledActor(),
				IOSTATE);
		}
		
		static IoObject *
		setCurrentActor(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("Game.setCurrentActor")
			IOASSERT(IoMessage_argCount(m) == 1,"Expected one argument")
			IoObject *arg = IoMessage_locals_objectArgAt_(m, locals, 0);
			getObject(self)->setCurrentlyControlledActor(
				unwrapObject<Ptr<IActor> >(arg));
			return self;
		}
		
	};
}

template<>
void addMapping<IGame>(Ptr<IGame> game, IoState *state) {
	GameMapping::addMapping(game,state);
}

template<>
Ptr<IGame> unwrapObject<Ptr<IGame> >(IoObject * self) {
	return (IGame*)self->data;
}

