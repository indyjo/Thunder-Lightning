#ifndef OBJECT_H
#define OBJECT_H

class Object;
template<class T> class Ptr;

#define DEBUG_OBJECTS 0

#ifdef DEBUG_OBJECTS
#include <typeinfo>
#endif

#ifndef DEBUG_OBJECTS
class Object {
    int refs;
public:
    inline Object() : refs(0) { }
    inline virtual ~Object() { };
    inline void ref() { refs++; };
    inline void unref() { refs--; if (refs == 0) delete this; };
};
#else
class Object {
protected:
    int refs;
public:
    inline Object() : refs(0) { }
    inline virtual ~Object() { };
    inline virtual void ref(void * p) { refs++; };
    inline virtual void unref(void * p) { refs--; if (refs == 0) delete this; };
};
class DebugObject : virtual public Object{
public:
    inline DebugObject() {
        ls_warning("DebugObject() called for %s object at %p\n",
                typeid(this).name(), this);
    }
    inline virtual ~DebugObject() {
        ls_warning("~DebugObject() called for %s object at %p\n",
                typeid(this).name(), this);
    };
    inline virtual void ref(void * p) {
        ls_warning("ref() called for %s object at %p by %p (-> refs=%d)\n",
                typeid(this).name(), this, p, refs+1);
        refs++;
    };
    inline virtual void unref(void * p) {
        ls_warning("unref() called for %s object at %p by %p (-> refs=%d)\n",
                typeid(this).name(), this, p, refs-1);
        refs--;
        if (refs == 0) {
            ls_warning("delete this!\n");
            delete this;
        }
    };
};
#endif

class SigObject: virtual public Object, virtual public SigC::Object { };

#ifndef DEBUG_OBJECTS
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
#else
template<class T> class Ptr {
    T * p;
public:
    inline Ptr() : p(0) {};
    inline Ptr(T * obj) : p(obj) {if (p) p->ref(this);};
    inline Ptr(const Ptr<T> & ptr) : p(ptr.p) {if (p) p->ref(this);};
    template<class U> inline Ptr(const Ptr<U> & ptr)
            : p(static_cast<T*>(&*ptr))
    {
        if (p) p->ref(this);
    };
    inline ~Ptr() {if (p) p->unref(this);};
    inline Ptr<T> & operator= (T * obj) {
        if(p) p->unref(this);
        p=obj;
        if(p) p->ref(this);
        return *this;
    };
    inline Ptr<T> & operator= (const Ptr<T> & ptr) {
        if(p) p->unref(this);
        p=ptr.p;
        if(p) p->ref(this);
        return *this;
    };
    template<class U> inline Ptr<T> & operator= (const Ptr<U> & ptr) {
        if (p) p->unref(this);
        p=static_cast<T*>(&*ptr);
        p->ref(this);
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

#endif
