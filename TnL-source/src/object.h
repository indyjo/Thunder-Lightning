#ifndef OBJECT_H
#define OBJECT_H
#include <sigc++/sigc++.h>

// Global switch to enable reference counting debugging
//#define OBJECT_DEBUG 1
#define OBJECT_DEBUG 0

class Object;
template<class T> class Ptr;

#if OBJECT_DEBUG
    #include <set>
    struct Context;
#endif

class Object {
    int refs;
public:
#if OBJECT_DEBUG
    typedef std::multiset<const Context *> References;
    References references;
    
	Object();
    virtual ~Object();
    void ref();
    void unref();
    void addReference(const Context *);
    void removeReference(const Context *);
#else
    inline Object() : refs(0) { }
    inline virtual ~Object() { };
    inline void ref() { refs++; };
    inline void unref() { refs--; if (refs == 0) delete this; };
#endif
    inline int getRefs() const { return refs; }
    
	static void debug();
	static void debug(Object*);
	static void backtrace();
};

class SigObject: virtual public Object, virtual public SigC::Object { };

#if OBJECT_DEBUG
    #define REF(p) do{ (p)->ref(); (p)->addReference(ctx); } while (false)
    #define UNREF(p) do{ (p)->removeReference(ctx); (p)->unref(); } while (false)
    
    struct DebugPtr {
        Context * ctx;
        DebugPtr();
        ~DebugPtr();
    };
#else
    #define REF(p) (p)->ref()
    #define UNREF(p) (p)->unref()
#endif

template<class T> class Ptr
#if OBJECT_DEBUG
    : private DebugPtr
#endif
{
    T * p;
public:
	friend T* ptr(const Ptr<T>& arg) {return arg.p;}
    
    inline Ptr() : p(0) {};
    inline Ptr(T * obj) : p(obj) {if (p) REF(p);};
    inline Ptr(const Ptr<T> & ptr) : p(ptr.p) {if (p) REF(p);};
    template<class U> inline Ptr(const Ptr<U> & other)
            : p(static_cast<T*>(ptr(other)))
    {
        if (p) REF(p);
    };
    inline ~Ptr() {if (p) UNREF(p);};
    inline Ptr<T> & operator= (T * obj) {
        if(obj) REF(obj);
        if(p) {
        	T *oldp = p;
        	p = obj;
        	UNREF(oldp);
        } else p=obj;
        return *this;
    };
    inline Ptr<T> & operator= (const Ptr<T> & ptr) {
        if(ptr.p) REF(ptr.p);
        if(p) {
        	T *oldp = p;
        	p = ptr.p;
        	UNREF(oldp);
        } else p=ptr.p;
        return *this;
    };
    template<class U> inline Ptr<T> & operator= (const Ptr<U> & other) {
        if (ptr(other)) REF(ptr(other));
        if(p) {
        	T *oldp = p;
        	p = static_cast<T*>(ptr(other));
        	UNREF(oldp);
        } else p = static_cast<T*>(ptr(other));
        return *this;
    };
    inline T & operator*  () const { return *p; };
    inline T * operator-> () const { return p; };
    /*template<class U> inline operator Ptr<U>() {
        return Ptr<U>(*this);
    };*/
    inline operator bool() const { return p!=0; }
    inline bool operator!() const { return p == 0; }
    inline bool operator==(const Ptr<T> & ptr) const { return p == ptr.p; }
    inline bool operator!=(const Ptr<T> & ptr) const { return p != ptr.p; }
    inline bool operator<(const Ptr<T> & ptr) const { return p < ptr.p; }
    inline bool operator<=(const Ptr<T> & ptr) const { return p <= ptr.p; }
    inline bool operator>(const Ptr<T> & ptr) const { return p > ptr.p; }
    inline bool operator>=(const Ptr<T> & ptr) const { return p >= ptr.p; }
private:
    // Disallow conversions to number types
    inline operator int() const { return 0; }
    inline operator unsigned int() const { return 0; }
    inline operator long() const { return 0; }
    inline operator unsigned long() const { return 0; }
    inline operator short() const { return 0; }
    inline operator unsigned short() const { return 0; }
    inline operator char() const { return 0; }
    inline operator unsigned char() const { return 0; }
    inline operator double() const { return 0; }
    inline operator float() const { return 0; }
};


#endif
