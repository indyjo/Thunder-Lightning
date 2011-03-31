#include <cstdio>

#include "object.h"

#if OBJECT_DEBUG

#include <set>
#include <map>
#include <string>
#include <cstdlib>
#include <typeinfo>
#include <cxxabi.h>
#include <execinfo.h>
#include <dlfcn.h>

using namespace std;

string demangle(const char *name) {
	if (!name) return "<null>";
	int status;
	char *demangled = abi::__cxa_demangle(name,NULL,NULL,&status);
	if (demangled && status==0) {
		string res = demangled;
		free(demangled);
		return res;
	}
	return name;
}
	

string getFunctionName(void * address) {
	char buf[1024];
	Dl_info info;
	if (dladdr(address, &info)) {
		snprintf(buf, 1024, "%s+0x%xd: %p",
			demangle(info.dli_sname).c_str(),
			(char*)address - (char*)info.dli_saddr, address);
	} else {
		snprintf(buf, 1024, "<unknown>: %p", address);
	}
	return buf;
}

struct Context {
	void **ctx;
	int size;
	Context() {
		size = 0;
	}
	Context(void **buf, int n) {
		ctx = new void*[n];
		for(int i=0; i<n; ++i) ctx[i] = buf[i];
		size = n;
	}
	Context(const Context & other) {
		size = other.size;
		ctx = new void*[size];
		for(int i=0; i<size; ++i) ctx[i] = other.ctx[i];
	}
	
	~Context() {
		if (size>0) delete [] ctx;
	}
	
	bool operator<(const Context & other) const {
		int n = std::min(size, other.size);
		for(int i=0; i<n; ++i) {
			if (ctx[i] < other.ctx[i]) return true;
			if (ctx[i] > other.ctx[i]) return false;
		}
		return size < other.size;
	}
	
	void operator=(const Context & other) {
		if (size > 0) delete[] ctx;
		size = other.size;
		ctx = new void*[size];
		for(int i=0; i<size; ++i) ctx[i] = other.ctx[i];
	}
	
	void dump(int indent = 2, FILE * f=stderr) const {
		for(int i=0; i<size; ++i) {
			for(int j=0; j<indent; ++j) fputc(' ',f);
			fprintf(f, "% 4d: %s\n", i, getFunctionName(ctx[i]).c_str());
		}
	}
		
};

void doBacktrace() {
	void *buf[256];
	int n = backtrace(buf, 256);
	for (int i=0; i<n; ++i) {
		printf("% 3d: %s\n", i, getFunctionName(buf[i]).c_str());
	}
}

Context get_context(int ofs=3, int size=20) {
	void *buf[256];
	int n = backtrace(buf, 256);
	if (n <= ofs) fprintf(stderr, "Error getting context\n");
	return Context(buf+ofs, std::min(size, n-ofs));
}

typedef set<Object *> ObjectSet;
typedef map<Object *, Context> SingleContextMap;
typedef map<Object *, set<Context> > ContextMap;
typedef map<pair<Object *,Context>, int> CounterMap;

	ObjectSet *_objects=0;
inline ObjectSet & objects()
{ return *(_objects?_objects:(_objects=new ObjectSet)); }

	SingleContextMap *_construct_contexts=0;
inline SingleContextMap & construct_contexts()
{ return *(_construct_contexts?_construct_contexts:(_construct_contexts=new SingleContextMap)); }

ContextMap *_context_map=0;
inline ContextMap & context_map()
{ return *(_context_map?_context_map:(_context_map=new ContextMap)); }

CounterMap *_counter_map=0;
inline CounterMap & counter_map()
{ return *(_counter_map?_counter_map:(_counter_map=new CounterMap)); }

Object::Object()
:	refs(0)
{
	objects().insert(this);
	construct_contexts()[this] = get_context();
}

Object::~Object()
{
	objects().erase(this);
	fprintf(stderr, "Num objects: %d\n", objects().size());
}

void Object::ref() { 
	refs++;
	Context ctx = get_context(2);
	context_map()[this].insert(ctx);
	counter_map()[make_pair(this,ctx)]++;
};
void Object::unref() {
	refs--;
	Context ctx = get_context(2);
	context_map()[this].insert(ctx);
	counter_map()[make_pair(this,ctx)]--;
	if (refs == 0) {
		printf("%s commits suicide.\n", demangle(typeid(*this).name()).c_str());
		delete this;
		printf("dead.\n");
	}
};

void Object::addReference(const Context * ctx) {
    references.insert(ctx);
}

void Object::removeReference(const Context * ctx) {
    References::iterator i = references.find(ctx);
    if ( i != references.end() ) {
        references.erase(i);
    } else {
        printf("Warning: addReference/removeReference mismatch!\n");
    }
}

void object_debuginfo_at_exit () __attribute__ ((destructor));
void object_debuginfo_at_exit () {
	Object::debug();
	printf("Done writing debug info stats\n");
}

#endif

void Object::debug() {
#if OBJECT_DEBUG
	typedef ObjectSet::iterator Iter;
	typedef set<Context>::iterator CtxIter;
	typedef Object::References::iterator RefsIter;
	int count=0;
	fprintf(stdout, "Printing debug info for %d objects.\n", objects().size());
	fprintf(stderr, "Printing debug info for %d objects.\n", objects().size());
	for(Iter i=objects().begin(); i!=objects().end(); ++i) {
		Object *o = *i;
		Context c = construct_contexts()[o];
		fprintf(stderr, "%d: %s at %p with %d refs created in:\n",
			count,
			demangle(typeid(*o).name()).c_str(),
			o,
			o->getRefs());
		c.dump();
		
		fprintf(stderr, "-- With references:\n");
		int rcount = 1;
		for (RefsIter i = o->references.begin(); i!=o->references.end(); ++i) {
		    fprintf(stderr, "  * ref %d created in:\n", rcount++);
		    (*i)->dump(4);
		}
		
		set<Context> contexts = context_map()[o];
		for(CtxIter j = contexts.begin(); j!= contexts.end(); ++j) {
			Context ctx = *j;
			int ccount = counter_map()[make_pair(o,ctx)];
			fprintf(stderr, "-- Count %d for context:\n", ccount);
			ctx.dump(4);
		}
		
		count++;
	}
#else
	fprintf(stderr, "Object Debugging not enabled.\n");
#endif
}

void Object::debug(Object *o) {
#if OBJECT_DEBUG
	typedef set<Context>::iterator CtxIter;
	Context c = construct_contexts()[o];
	fprintf(stderr, "%s at %p with %d refs created in:\n",
		demangle(typeid(*o).name()).c_str(),
		o,
		o->getRefs());
	c.dump();
	
	set<Context> contexts = context_map()[o];
	for(CtxIter j = contexts.begin(); j!= contexts.end(); ++j) {
		Context ctx = *j;
		int ccount = counter_map()[make_pair(o,ctx)];
		fprintf(stderr, "-- Count %d for context:\n", ccount);
		ctx.dump(4);
	}
#else
	fprintf(stderr, "Object Debugging not enabled.\n");
#endif
}

void Object::backtrace() {
#if OBJECT_DEBUG
	doBacktrace();
#else
	printf("Object Debugging not enabled.\n");
#endif
}

#if OBJECT_DEBUG
DebugPtr::DebugPtr() {
    ctx = new Context(get_context());
}

DebugPtr::~DebugPtr() {
    delete ctx;
}
#endif
