#ifndef VOIDPTR_H
#define VOIDPTR_H

#include "object.h"

class VoidPtr {
    Object *p_obj;
    void   *p_void;

public:
    inline VoidPtr() : p_obj(0), p_void(0) { }
    inline VoidPtr(const VoidPtr & vp) : p_obj(vp.p_obj), p_void(vp.p_void) {
        safeRef();
    }
    template<class T> inline VoidPtr(T *p) {
        p_obj  = static_cast<Object*>(p);
        p_void = reinterpret_cast<void *>(p);
        safeRef();
    }
    template<class T> inline VoidPtr(Ptr<T> p) {
        p_obj  = static_cast<Object*>( & *p);
        p_void = reinterpret_cast<void*>( & *p);
        safeRef();
    }
    inline ~VoidPtr() {
        safeUnref();
    }

    inline VoidPtr & operator= (const VoidPtr & vp) {
        Object * old_p_obj = p_obj;
        p_obj = vp.p_obj;
        p_void = vp.p_void;
        safeRef();
        if (old_p_obj) old_p_obj->unref();
        return *this;
    }

    inline template<class T> operator T * () const {
        return reinterpret_cast<T *> p_void;
    }

    inline template<class T> T dynamic_cast() {
        return dynamic_cast<T>(p_void);
    }
    
protected:
    void safeRef()   { if(p_obj) p_obj->ref(); }
    void safeUnref() { if(p_obj) p_obj->unref(); }
};


#endif