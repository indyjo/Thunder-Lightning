#ifndef SCRIPTING_MAPPINGS_H
#define SCRIPTING_MAPPINGS_H

#include <interfaces/IGame.h>
#include "IoIncludes.h"
extern "C" {
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

/// Add all type mappings to state
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

#endif
