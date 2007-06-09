// ---------------------------------------------------------------
// |  PrimitiveMappings                                          |
// ---------------------------------------------------------------


#include <string>
#include <vector>
#include <tnl.h>
#include <interfaces/IActor.h>

#include "mappings.h"

#define IOASS(expr, message) \
	if (!(expr))    \
		IoState_error_(IOSTATE, NULL, "Io.Assertion '%s'", message);

using namespace std;

template<> IoObject * wrapObject(bool b, IoState * state)
{ return b?state->ioTrue:state->ioFalse; }
template<> IoObject * wrapObject(int n, IoState * state)
{ return IoNumber_newWithDouble_(state, n); }
template<> IoObject * wrapObject(float n, IoState * state)
{ return IoNumber_newWithDouble_(state, n); }
template<> IoObject * wrapObject(double n, IoState * state)
{ return IoNumber_newWithDouble_(state, n); }
template<> IoObject * wrapObject(std::string s, IoState * state)
{ return IoSeq_newWithCString_(state, const_cast<char*>(s.c_str())); }
template<> IoObject * wrapObject(const char * s, IoState * state)
{ return IoSeq_newWithCString_(state, const_cast<char*>(s)); }
template<> IoObject * wrapObject(char * s, IoState * state)
{ return IoSeq_newWithCString_(state, s); }

template<> bool unwrapObject(IoObject *self)
{ return ISTRUE(self); }
template<> int unwrapObject(IoObject *self)
{ return IoNumber_asInt(self); }
template<> float unwrapObject(IoObject *self)
{ return IoNumber_asDouble(self); }
template<> double unwrapObject(IoObject *self)
{ return IoNumber_asDouble(self); }
template<> std::string unwrapObject(IoObject *self)
{ return CSTRING(self); }
template<> const char* unwrapObject(IoObject *self)
{ return CSTRING(self); }
template<> char* unwrapObject(IoObject *self)
{ return CSTRING(self); }

template<> bool isA<std::string>(IoObject *self) { return ISSEQ(self); }

namespace {
	template<class T>
	inline IoObject *wrap_vector(const vector<T> & v, IoState *state) {
		IoList * list = IoList_new(state);
		for(int i=0; i<v.size(); ++i)
			IoList_rawAppend_(list, wrapObject<T>(v[i], state));
		return list;
	}

	template<class T>
	inline vector<T> unwrap_vector(IoObject *self) {
		IOASS(ISLIST(self), "Not a list")
		int n=(int)IoList_rawSize(self);
		vector<T> v;
		for(int i=0; i<n; ++i)
			v.push_back(unwrapObject<T>(IoList_rawAt_(self, i)));
		return v;
	}
}


template<> IoObject * wrapObject(vector<Ptr<IActor> > v, IoState * state)
{ return wrap_vector(v,state); }
template<> IoObject * wrapObject(vector<string> v, IoState * state)
{ return wrap_vector(v,state); }

template<> vector<Ptr<IActor> > unwrapObject(IoObject *self)
{ return unwrap_vector<Ptr<IActor> >(self); }
template<> vector<string> unwrapObject(IoObject *self)
{ return unwrap_vector<string>(self); }
