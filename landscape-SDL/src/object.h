#ifndef OBJECT_H
#define OBJECT_H
#include <sigc++/sigc++.h>

//#define OBJECT_DEBUG 1
#define OBJECT_DEBUG 0

class Object;
template<class T> class Ptr;


class Object {
    int refs;
public:
#if OBJECT_DEBUG
	Object();
    virtual ~Object();
    void ref();
    void unref();
#else
    inline Object() : refs(0) { }
    inline virtual ~Object() { };
    inline void ref() { refs++; };
    inline void unref() { refs--; if (refs == 0) delete this; };
#endif
    inline int getRefs() const { return refs; }
    
	static void debug();
	static void backtrace();
};

class SigObject: virtual public Object, virtual public SigC::Object { };

template<class T>
T* ptr(const Ptr<T> & arg) { return arg.p; }

template<class T> class Ptr {
    T * p;
public:
	friend T* ptr<T>(const Ptr<T>&);
    
    inline Ptr() : p(0) {};
    inline Ptr(T * obj) : p(obj) {if (p) p->ref();};
    inline Ptr(const Ptr<T> & ptr) : p(ptr.p) {if (p) p->ref();};
    template<class U> inline Ptr(const Ptr<U> & other)
            : p(static_cast<T*>(ptr(other)))
    {
        if (p) p->ref();
    };
    inline ~Ptr() {if (p) p->unref();};
    inline Ptr<T> & operator= (T * obj) {
        if(obj) obj->ref();
        if(p) {
        	T *oldp = p;
        	p = obj;
        	oldp->unref();
        } else p=obj;
        return *this;
    };
    inline Ptr<T> & operator= (const Ptr<T> & ptr) {
        if(ptr.p) ptr.p->ref();
        if(p) {
        	T *oldp = p;
        	p = ptr.p;
        	oldp->unref();
        } else p=ptr.p;
        return *this;
    };
    template<class U> inline Ptr<T> & operator= (const Ptr<U> & other) {
        if (ptr(other)) ptr(other)->ref();
        if(p) {
        	T *oldp = p;
        	p = static_cast<T*>(ptr(other));
        	oldp->unref();
        } else p = static_cast<T*>(ptr(other));
        return *this;
    };
    inline T & operator*  () const { return *p; };
    inline T * operator-> () const { return p; };
    /*template<class U> inline operator Ptr<U>() {
        return Ptr<U>(*this);
    };*/
    inline operator bool() { return p!=0; }
    inline bool operator==(const Ptr<T> & ptr) const { return p == ptr.p; }
    inline bool operator!=(const Ptr<T> & ptr) const { return p != ptr.p; }
    inline bool operator<(const Ptr<T> & ptr) const { return p < ptr.p; }
    inline bool operator<=(const Ptr<T> & ptr) const { return p <= ptr.p; }
    inline bool operator>(const Ptr<T> & ptr) const { return p > ptr.p; }
    inline bool operator>=(const Ptr<T> & ptr) const { return p >= ptr.p; }
};


#endif
