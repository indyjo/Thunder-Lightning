#ifndef SCRIPTING_MAPPINGS_H
#define SCRIPTING_MAPPINGS_H

#include <interfaces/IGame.h>
#include "IoIncludes.h"
extern "C" {
	#include <IoVM/IoList.h>
	#include <IoVM/IoMessage.h>
	#include <IoVM/IoNil.h>
	#include <IoVM/IoNumber.h>
}

#define BEGIN_FUNC(name) 							\
	IOASSERT(self->data, "Pointer to C++ Object is zero")

#define VOID_FUNC(funcname)							\
	static IoObject * funcname (						\
	IoObject *self, IoObject *locals, IoMessage *m) {	\
		BEGIN_FUNC(#funcname)							\
		getObject(self)-> funcname ();					\
		return self;									\
	}


#define GET_NUMBER(funcname)							\
	static IoObject * funcname (						\
	IoObject *self, IoObject *locals, IoMessage *m) {	\
		BEGIN_FUNC(#funcname)							\
		return IONUMBER(getObject(self)-> funcname ());	\
	}

#define SET_INT(funcname)								\
	static IoObject * funcname (						\
	IoObject *self, IoObject *locals, IoMessage *m) {	\
		BEGIN_FUNC(#funcname)							\
		IOASSERT(IoMessage_argCount(m) == 1,			\
			"Expected one argument")					\
		IoObject *arg = IoMessage_locals_numberArgAt_(	\
			m, locals, 0);								\
		getObject(self)-> funcname(IoNumber_asInt(arg));\
		return self;									\
	}

#define SET_ENUM(funcname, enum_name)					\
	static IoObject * funcname (						\
	IoObject *self, IoObject *locals, IoMessage *m) {	\
		BEGIN_FUNC(#funcname)							\
		IOASSERT(IoMessage_argCount(m) == 1,			\
			"Expected one argument")					\
		IoObject *arg = IoMessage_locals_numberArgAt_(	\
			m, locals, 0);								\
		getObject(self)-> funcname(						\
			(enum_name) IoNumber_asInt(arg));			\
		return self;									\
	}


#define SET_FLOAT(funcname)								\
	static IoObject * funcname (						\
	IoObject *self, IoObject *locals, IoMessage *m) {	\
		BEGIN_FUNC(#funcname)							\
		IOASSERT(IoMessage_argCount(m) == 1,			\
			"Expected one argument")					\
		IoObject *arg = IoMessage_locals_numberArgAt_(	\
			m, locals, 0);								\
		getObject(self)-> funcname (					\
			(float)IoNumber_asDouble(arg));				\
		return self;									\
	}

#define GET_BOOLEAN(funcname)							\
	static IoObject * funcname(							\
	IoObject *self, IoObject *locals, IoMessage *m) {	\
		BEGIN_FUNC(#funcname)							\
		if (getObject(self)->funcname()) {				\
			return self;								\
		} else {										\
			return IONIL(self);							\
		}												\
	}

#define GET_VECTOR(funcname)							\
	static IoObject * funcname(							\
	IoObject *self, IoObject *locals, IoMessage *m) {	\
		BEGIN_FUNC(#funcname)							\
		return wrapObject<Vector>(					    \
			getObject(self)->funcname(), IOSTATE);  	\
	}

#define SET_VECTOR(funcname)							\
	static IoObject * funcname(							\
	IoObject *self, IoObject *locals, IoMessage *m) {	\
		IOASSERT(IoMessage_argCount(m) == 1,			\
			"Expected one argument")					\
		IoObject *arg = IoMessage_locals_valueArgAt_(	\
			m, locals, 0);								\
		getObject(self)-> funcname (					\
			unwrapObject<Vector>(arg));					\
		return self;									\
	}

#define SETTER(type, funcname)							\
	static IoObject * funcname(IoObject *self, IoObject *locals, IoMessage *m){	\
		IOASSERT(IoMessage_argCount(m) == 1, "Expected one argument")			\
		IoObject *arg = IoMessage_locals_valueArgAt_(m,locals,0);				\
		getObject(self)-> funcname( unwrapObject<type>(arg) );					\
		return self;															\
	}

#define GETTER(type, funcname)													\
	static IoObject * funcname(IoObject *self, IoObject *locals, IoMessage *m){	\
		BEGIN_FUNC(#funcname)													\
		return wrapObject<type>( getObject(self)-> funcname(), IOSTATE );		\
	}
	
#define GET_STRING(funcname) GETTER(std::string, funcname)
#define SET_STRING(funcname) SETTER(std::string, funcname)

/// Add all type mappings to state
void addBasicMappings(Ptr<IGame>, IoState *);
void addMappings(Ptr<IGame>, IoState *);

/// Template definition for mapping of type T
template<class T>
void addMapping(Ptr<IGame> game, IoState *state);

/// Template definition to obtain a new Object given a pointer
template<class T>
IoObject * wrapObject(T, IoState *);

/// Template definition to obtain a pointer to a C++ Object given an Object
template<class T>
T unwrapObject(IoObject *);

/// Template definition to obtain the proto of a type mapping
template<class T>
IoObject * getProtoObject(IoState *);

template<class S, class T>
IoObject * castfunc(IoObject * self, IoObject*, IoObject*) {
	return wrapObject<T>(unwrapObject<S>(self), IOSTATE);
}

template<class T>
struct TemplatedObjectMapping {
	static T* getObject(IoObject *o)
	{ return ((T*) o->data); }
	
	static IoTag *tag(void * state, char * name) {
	    IoTag *tag = IoTag_newWithName_(name);
	    tag->state = state;
	    tag->cloneFunc = (TagCloneFunc *)rawClone;
	    tag->markFunc  = (TagMarkFunc *)mark;
	    tag->freeFunc  = (TagFreeFunc *)free;
	    //tag->writeToStoreFunc  = (TagWriteToStoreFunc *)IoFile_writeToStore_;
	    //tag->readFromStoreFunc = (TagReadFromStoreFunc *)IoFile_readFromStore_;
	    return tag;
	}
	
	static IoObject * rawClone(IoObject *self) 
	{ 
		IoObject *child = IoObject_rawClonePrimitive(self);
		if (self->data) getObject(self)->ref();
		child->data = self->data;
		return child;
	}
	
	static void mark(IoObject * self) {
		//if (self->data) getObject(self)->ref();
	}
	static void free(IoObject * self) {
		if (self->data) getObject(self)->unref();
	}
	static void retarget(IoObject *self, T *target) {
		if (target) target->ref();
		if (self->data) {
			T* old_target = (T*) self->data;
			self->data = &*target;
			old_target->unref();
		} else self->data = &*target;
	}
};

#define CREATE_FUNC(T)														\
	static IoObject * create(Ptr<T> p, IoState *state) 						\
	{																		\
		IoObject *child = IoObject_rawClonePrimitive(						\
			IoState_protoWithInitFunction_(state, proto));					\
		retarget(child, ptr(p));											\
		return child;														\
	}
	

#endif
