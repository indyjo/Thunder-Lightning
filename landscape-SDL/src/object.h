#ifndef OBJECT_H
#define OBJECT_H
#include <sigc++/sigc++.h>

class Object;
template<class T> class Ptr;

class Object {
    int refs;
public:
    inline Object() : refs(0) { }
    inline virtual ~Object() { };
    inline void ref() { refs++; };
    inline void unref() { refs--; if (refs == 0) delete this; };
};

class SigObject: virtual public Object, virtual public SigC::Object { };

template<class T> class Ptr {
    T * p;
public:
    inline Ptr() : p(0) {};
    inline Ptr(T * obj) : p(obj) {if (p) p->ref();};
    inline Ptr(const Ptr<T> & ptr) : p(ptr.p) {if (p) p->ref();};
    template<class U> inline Ptr(const Ptr<U> & ptr)
            : p(static_cast<T*>(&*ptr))
    {
        if (p) p->ref();
    };
    inline ~Ptr() {if (p) p->unref();};
    inline Ptr<T> & operator= (T * obj) {
        if(p) p->unref();
        p=obj;
        if(p) p->ref();
        return *this;
    };
    inline Ptr<T> & operator= (const Ptr<T> & ptr) {
        if(p) p->unref();
        p=ptr.p;
        if(p) p->ref();
        return *this;
    };
    template<class U> inline Ptr<T> & operator= (const Ptr<U> & ptr) {
        if (p) p->unref();
        p=static_cast<T*>(&*ptr);
        p->ref();
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
