// ---------------------------------------------------------------
// |  ConfigMapping                                              |
// ---------------------------------------------------------------

#include <interfaces/IConfig.h>
#include "mappings.h"

namespace {
	
	struct ConfigMapping {
		static IoObject * config;
	
		static IConfig* getObject(IoObject *a) { return ((IConfig*)a->data); }
		
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject *object = state->mainActor;
			IoObject *lobby = state->lobby;
			
			config = proto(state);
			IoObject_setSlot_to_(lobby,
				IoState_stringWithCString_(state, "Config"), config);
			config->data = &*thegame->getConfig();
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
	
		static IoObject *proto(IoState *state) {
			IoMethodTable methodTable[] = {
				/* standard I/O */
				{"query", query},
				{"set", set},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			self->tag = tag(state, "Config");
			
			self->data = 0;
			IoState_registerProtoWithFunc_(state, self,
				(IoStateProtoFunc*) proto);
			
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		static void mark(IoObject * self) {
			if (self->data) getObject(self)->ref();
		}
		static void free(IoObject * self) {
			if (self->data) getObject(self)->unref();
		}
		
		static IoObject *rawClone(IoObject *self) 
		{ 
			IoObject *child = IoObject_rawClonePrimitive(self);
			child->data = self->data;
			return child;
		}
		
		static IoObject * query
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("Config.query")
			IOASSERT(IoMessage_argCount(m) == 1,"Expected one argument")
			char * key = IoMessage_locals_cStringArgAt_(m, locals, 0);
			char valuebuf[256];
			const char *val = getObject(self)->query(key);
			if (val) {
				strncpy(valuebuf, val, 256);
				return IOSTRING(valuebuf);
			} else {
				return IONIL(self);
			}
		}
		
		static IoObject * set
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("Config.set")
			IOASSERT(IoMessage_argCount(m) == 2,"Expected two arguments")
			char * key = IoMessage_locals_cStringArgAt_(m, locals, 0);
			char * val = IoMessage_locals_cStringArgAt_(m, locals, 1);
			getObject(self)->set(key,val);
			return self;
		}
	};
	
	IoObject * ConfigMapping::config;
}

template<>
void addMapping<IConfig>(Ptr<IGame> game, IoState *state) {
	ConfigMapping::addMapping(game,state);
}

template<>
IoObject * wrapObject<Ptr<IConfig> >(Ptr<IConfig> config, IoState *) {
	IoObject *new_object = ConfigMapping::rawClone(ConfigMapping::config);
	new_object->data = &*config;
	return new_object;
}

template<>
Ptr<IConfig> unwrapObject<Ptr<IConfig> >(IoObject * self) {
	return (IConfig*)self->data;
}

