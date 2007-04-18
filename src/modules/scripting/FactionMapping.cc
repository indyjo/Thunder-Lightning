// ---------------------------------------------------------------
// |  FactionMapping                                             |
// ---------------------------------------------------------------

#include <Faction.h>
#include "mappings.h"

template<>
Ptr<Faction> unwrapObject(IoObject * self) {
	return (Faction*)IoObject_dataPointer(self);
}


namespace {
	
	struct FactionMapping : public TemplatedObjectMapping<Faction> {
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject *lobby = state->lobby;
			
			IoObject *self = proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(lobby, IOSYMBOL("Faction"), self);
		}
		
		static IoTag *tag(void * state, char * name) {
		    IoTag *tag = IoTag_newWithName_(name);
		    tag->state = state;
		    tag->cloneFunc = (IoTagCloneFunc *)rawClone;
		    tag->markFunc  = (IoTagMarkFunc *)mark;
		    tag->freeFunc  = (IoTagFreeFunc *)free;
		    //tag->writeToStoreFunc  = (TagWriteToStoreFunc *)IoFile_writeToStore_;
		    //tag->readFromStoreFunc = (TagReadFromStoreFunc *)IoFile_readFromStore_;
		    return tag;
		}
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"getName", getName},
				{"setName", setName},
				{"getDefaultAttitude", getDefaultAttitude},
				{"setDefaultAttitude", setDefaultAttitude},
				{"getAttitudeTowards", getAttitudeTowards},
				{"setAttitudeTowards", setAttitudeTowards},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "Faction"));
			IoObject_setDataPointer_(self, 0);
			retarget(self, new Faction);
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
	
		static IoObject * create(Ptr<Faction> faction, IoState *state) 
		{
			IoObject *child = IOCLONE(
				IoState_protoWithInitFunction_(state, proto));
			retarget(child, ptr(faction));
			return child;
		}
			
		static IoObject * rawClone(IoObject *self) 
		{ 
			IoObject *child = IoObject_rawClonePrimitive(self);
			retarget(child, new Faction);
			*getObject(child) = *getObject(self);
			return child;
		}
		
		static IoObject * getAttitudeTowards
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("Faction.getAttitudeTowards")
			IOASSERT(IoMessage_argCount(m) == 1,"Expected one argument")
			Ptr<Faction> other = unwrapObject<Ptr<Faction> > (
				IoMessage_locals_valueArgAt_(m, locals, 0));
			return wrapObject<int>(
				getObject(self)->getAttitudeTowards(other), IOSTATE);
		}

		static IoObject * setAttitudeTowards
		(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("Faction.setAttitudeTowards")
			IOASSERT(IoMessage_argCount(m) == 2,"Expected two arguments")
			Ptr<Faction> other = unwrapObject<Ptr<Faction> > (
				IoMessage_locals_valueArgAt_(m, locals, 0));
			int attitude = unwrapObject<int> (
				IoMessage_locals_valueArgAt_(m, locals, 1));
			getObject(self)->setAttitudeTowards(
				other,
				(Faction::Attitude) attitude);
			return self;
		}
		
		GET_STRING(getName)
		SET_STRING(setName)
		GET_NUMBER(getDefaultAttitude)
		SET_ENUM(setDefaultAttitude, Faction::Attitude)
	};
}

template<>
void addMapping<Faction>(Ptr<IGame> game, IoState *state) {
	FactionMapping::addMapping(game,state);
}

template<>
IoObject * 
wrapObject<Ptr<Faction> >(Ptr<Faction> faction, IoState * state) {
	return FactionMapping::create(faction, state);
}

template<>
IoObject *getProtoObject<Ptr<Faction> >(IoState * state) {
	return IoState_protoWithInitFunction_(state, FactionMapping::proto);
}
