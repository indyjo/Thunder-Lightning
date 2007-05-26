#include <landscape.h>
#include <modules/math/SpecialMatrices.h>
#include <modules/math/MatrixVector.h>

#include <List.h>

#include "mappings.h"

#define LISTIVAR(self) ((List *)IoObject_dataPointer(self))

#define IOASS(expr, message) \
	if (!(expr))    \
		IoState_error_(IOSTATE, NULL, "Io Assertion '%s'", message);

namespace {
template<class T>
IoObject * wrap_raw(const T *v, int rows, int cols, IoState * state) {
    IoState_pushRetainPool(state);
    IoObject *self = IOCLONE(
        IoObject_getSlot_(state->lobby,
            IoState_symbolWithCString_(state, "Matrix")));

    IoObject_initClone_(state->lobby, state->lobby, NULL, self);
    
    IoList *entries = IoObject_rawGetSlot_(self, IOSYMBOL("entries"));
    List_preallocateToSize_(LISTIVAR(entries), rows*cols);
    
    for(int i=0; i<rows*cols; ++i) 
        IoList_rawAt_put_(entries, i, IONUMBER(v[i]));
    
    IoObject_setSlot_to_(self, IOSYMBOL("rows"), IONUMBER(rows));
    IoObject_setSlot_to_(self, IOSYMBOL("columns"), IONUMBER(cols));
    
    IoState_popRetainPoolExceptFor_(state,self);
    return self;
}

template<class T>
void unwrap_raw(IoObject *self, T *out, int rows, int cols) {
    IoState_pushRetainPool(IOSTATE);
    IoObject *matrix = IoObject_getSlot_(IOSTATE->lobby, IOSYMBOL("Matrix"));
    //ls_message("Got matrix? Object: %p\n", matrix);
    IOASS(matrix, "Could not find Matrix proto.")
    IOASS(IoObject_rawHasProto_(self, matrix), "Not a Matrix object")
    IOASS(IoNumber_asInt(IoObject_getSlot_(self,IOSYMBOL("rows"))) == rows
        && IoNumber_asInt(IoObject_getSlot_(self,IOSYMBOL("columns"))) == cols,
        "Wrong dimension")
    IoObject *entries = IoObject_getSlot_(self, IOSYMBOL("entries"));
    IOASS(entries && ISLIST(entries), "entries not found or invalid")
    for (int i=0; i<rows*cols; ++i)
	    out[i] = IoNumber_asDouble(IoList_rawAt_(entries,i));
    IoState_popRetainPool(IOSTATE);
}

} // namespace

template<> IoObject * wrapObject(Vector2 v, IoState * state)
{ return wrap_raw(v.raw(), 2, 1, state); }
template<> IoObject * wrapObject(Vector3 v, IoState * state)
{ return wrap_raw(v.raw(), 3, 1, state); }
template<> IoObject * wrapObject(Vector4 v, IoState * state)
{ return wrap_raw(v.raw(), 4, 1, state); }

template<> IoObject * wrapObject(Matrix2 M, IoState * state)
{ return wrap_raw(M.raw(), 2, 2, state); }
template<> IoObject * wrapObject(Matrix3 M, IoState * state)
{ return wrap_raw(M.raw(), 3, 3, state); }
template<> IoObject * wrapObject(Matrix4 M, IoState * state)
{ return wrap_raw(M.raw(), 4, 4, state); }

template<> Vector2 unwrapObject<Vector2>(IoObject *self) {
	float v[2];
	unwrap_raw(self, v, 2, 1);
	return Vector2(v);
}

template<> Vector3 unwrapObject<Vector3>(IoObject *self) {
	float v[3];
	unwrap_raw(self, v, 3, 1);
	return Vector3(v);
}

template<> Vector4 unwrapObject<Vector4>(IoObject *self) {
	float v[4];
	unwrap_raw(self, v, 4, 1);
	return Vector4(v);
}

template<> Matrix2 unwrapObject(IoObject *self) {
	float v[2*2];
	unwrap_raw(self, v, 2, 2);
	return Matrix2::Array(v);
}

template<> Matrix3 unwrapObject(IoObject *self) {
	float v[3*3];
	unwrap_raw(self, v, 3, 3);
	return Matrix3::Array(v);
}

template<> Matrix4 unwrapObject(IoObject *self) {
	float v[4*4];
	unwrap_raw(self, v, 4, 4);
	return Matrix4::Array(v);
}
