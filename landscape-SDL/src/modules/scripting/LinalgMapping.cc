#include <landscape.h>
#include <modules/math/SpecialMatrices.h>
#include <modules/math/MatrixVector.h>

#include "mappings.h"

#define IOASS(expr, message) \
	if (!(expr))    \
		IoState_error_description_(state, NULL, "Io.Assertion", message);

template<>
IoObject * wrapObject<Vector>(Vector v, IoState * state) {
	IoObject * lobby = state->lobby;
	IoObject *matrix = IoObject_rawClone(
		IoObject_getSlot_(lobby,
			IoState_stringWithCString_(state, "Matrix")));

	IoObject_initClone_(lobby, lobby, NULL, matrix);
	
	IoMessage *dimMessage = IoMessage_newWithName_(state,
		IoState_stringWithCString_(state, "dim"));
	IoMessage_setCachedArg_toInt_(dimMessage, 0, 3); ///< 3 rows
	IoMessage_setCachedArg_toInt_(dimMessage, 1, 1); ///< 1 column
	IoObject *result = IoMessage_locals_performOn_(dimMessage,lobby,matrix);
	
	IoMessage *setMessage = IoMessage_newWithName_(state,
		IoState_stringWithCString_(state, "set"));
	IoMessage_setCachedArg_to_(setMessage, 0,
		IoNumber_newWithDouble_(state, v[0]));
	IoMessage_setCachedArg_to_(setMessage, 1,
		IoNumber_newWithDouble_(state, v[1]));
	IoMessage_setCachedArg_to_(setMessage, 2,
		IoNumber_newWithDouble_(state, v[2]));
	result = IoMessage_locals_performOn_(setMessage,lobby,matrix);
	
	return matrix;
}	
	
