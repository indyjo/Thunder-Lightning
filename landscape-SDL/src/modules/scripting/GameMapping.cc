// ---------------------------------------------------------------
// |  GameMapping                                                |
// ---------------------------------------------------------------

#include <interfaces/IActor.h>
#include "mappings.h"

namespace {
	
	struct GameMapping {
		typedef IoObject IoGame;
		
		static IoGame * game;
	
		static IGame* getObject(IoGame *a) { return ((IGame*)a->data); }
		
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject *object = state->mainActor;
			IoObject *lobby = state->lobby;
			
			game = proto(state);
			IoObject_setSlot_to_(lobby,
				IoState_stringWithCString_(state, "Game"), game);
			game->data = &*thegame;
		}
		
		static IoTag *tag(IoState * state, char * name) {
		    IoTag *tag = IoTag_newWithName_(name);
		    tag->state = state;
		    tag->cloneFunc = (TagCloneFunc *)rawClone;
		    tag->markFunc  = (TagMarkFunc *)mark;
		    tag->freeFunc  = (TagFreeFunc *)free;
		    //tag->writeToStoreFunc  = (TagWriteToStoreFunc *)IoFile_writeToStore_;
		    //tag->readFromStoreFunc = (TagReadFromStoreFunc *)IoFile_readFromStore_;
		    return tag;
		}
	
		static IoGame *proto(IoState *state) {
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
	
		static void mark(IoGame * self) {
			if (self->data) getObject(self)->ref();
		}
		static void free(IoGame * self) {
			if (self->data) getObject(self)->unref();
		}
		
		static IoGame *rawClone(IoGame *self) 
		{ 
			IoObject *child = IoObject_rawClonePrimitive(self);
			child->data = self->data;
			return child;
		}
		
		static IoObject *
		getCurrentActor(IoGame *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("Game.getCurrentActor")
			return wrapObject<Ptr<IActor> >(
				getObject(self)->getCurrentlyControlledActor(),
				IOSTATE);
		}
		
		static IoObject *
		setCurrentActor(IoGame *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("Game.setCurrentActor")
			IOASSERT(IoMessage_argCount(m) == 1,"Expected one argument")
			IoObject *arg = IoMessage_locals_objectArgAt_(m, locals, 0);
			getObject(self)->setCurrentlyControlledActor(
				unwrapObject<Ptr<IActor> >(arg));
			return self;
		}
		
	};
	
	GameMapping::IoGame * GameMapping::game;
}

template<>
void addMapping<IGame>(Ptr<IGame> game, IoState *state) {
	GameMapping::addMapping(game,state);
}

template<>
Ptr<IGame> unwrapObject<Ptr<IGame> >(IoObject * self) {
	return (IGame*)self->data;
}

