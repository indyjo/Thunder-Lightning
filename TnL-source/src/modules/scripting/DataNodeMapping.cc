// ---------------------------------------------------------------
// |  DataNodeMapping                                            |
// ---------------------------------------------------------------

#include <DataNode.h>
#include "mappings.h"

#define GETSET(CTYPE, GETFUNC, SETFUNC, HASFUNC)                               \
		static IoObject * GETFUNC                                              \
		(IoObject *self, IoObject *locals, IoMessage *m) {                     \
			BEGIN_FUNC("DataNode." #GETFUNC)                                   \
			IOASSERT(IoMessage_argCount(m) == 1,"Expected one argument")       \
			char * key = IoMessage_locals_cStringArgAt_(m, locals, 0);         \
			if (getObject(self)->HASFUNC(key)) {                               \
			    return wrapObject(getObject(self)->GETFUNC(key), IOSTATE);     \
			} else {                                                           \
			    return IONIL(self);                                            \
			}                                                                  \
		}                                                                      \
		static IoObject * SETFUNC                                              \
		(IoObject *self, IoObject *locals, IoMessage *m) {                     \
			BEGIN_FUNC("DataNode" #SETFUNC)                                    \
			IOASSERT(IoMessage_argCount(m) == 2,"Expected two arguments")      \
			char * key = IoMessage_locals_cStringArgAt_(m, locals, 0);         \
			CTYPE val = unwrapObject<CTYPE>(                                   \
			    IoMessage_locals_valueArgAt_(m, locals, 1));                   \
			getObject(self)->SETFUNC(key,val);                                 \
			return self;                                                       \
		}

namespace {
	
	struct DataNodeMapping : public TemplatedObjectMapping<DataNode> {
		static void addMapping(Ptr<IGame> thegame, IoState * state) {
			IoObject *lobby = state->lobby;
			
			IoObject *self = proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(lobby, IOSYMBOL("DataNode"), self);
			retarget(self, new DataNode);
		}
		
		TAG_FUNC
		
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"bool", getBool},
				{"int", getInt},
				{"float", getFloat},
				{"string", getString},
				{"vector", getVector},
				{"setBool", setBool},
				{"setInt", setInt},
				{"setFloat", setFloat},
				{"setString", setString},
				{"setVector", setVector},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "DataNode"));
			IoObject_setDataPointer_(self, 0);
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
		
		static IoObject * rawClone(IoObject *self) 
		{ 
			IoObject *child = IoObject_rawClonePrimitive(self);
			retarget(child, new DataNode);
			*getObject(child) = *getObject(self);
			return child;
		}
	
		GETSET(bool, getBool, setBool, hasBool)
		GETSET(int, getInt, setInt, hasInt)
		GETSET(float, getFloat, setFloat, hasFloat)
		GETSET(std::string, getString, setString, hasString)
		GETSET(Vector, getVector, setVector, hasVector)

	};
}

template<>
void addMapping<DataNode>(Ptr<IGame> game, IoState *state) {
	DataNodeMapping::addMapping(game,state);
}

template<>
IoObject * 
wrapObject<Ptr<DataNode> >(Ptr<DataNode> node, IoState * state) {
	IoObject *new_object = IOCLONE(
		IoState_protoWithInitFunction_(state, DataNodeMapping::proto));
	DataNodeMapping::retarget(new_object, ptr(node));
	return new_object;
}

template<>
Ptr<DataNode> unwrapObject<Ptr<DataNode> >(IoObject * self) {
	return (DataNode*)IoObject_dataPointer(self);
}

