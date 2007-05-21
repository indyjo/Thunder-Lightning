#ifndef TNL_WEAK_H
#define TNL_WEAK_H

#include "object.h"

struct WeakPeer : public Object {
    bool object_alive;
    inline WeakPeer() : object_alive(true) { }
};

class Weak {
    Ptr<WeakPeer> peer;
public:
    inline Weak() { }
    inline virtual ~Weak() { if(peer) peer->object_alive=false; };
    inline Ptr<WeakPeer> getPeer() const {
        if (!peer) {
            Weak *volatile_this = const_cast<Weak*>(this);
            volatile_this->peer = new WeakPeer;
        }
        return peer;
    }
};


template<class T> class WeakPtr {
    T * p;
    Ptr<WeakPeer> peer;
public:
    friend T* ptr(const WeakPtr<T>& arg) {return arg.p;}

    inline WeakPtr() : p(0) {}
    template<class U> inline WeakPtr(U * obj)
            : p(static_cast<T*>(obj)), peer(obj->getPeer()) { }
    template<class U> inline WeakPtr(const WeakPtr<U> & other)
            : p(static_cast<T*>(other.p)), peer(other.peer) { }
    template<class U> inline WeakPtr(const Ptr<U> & other)
            : p(static_cast<T*>(ptr(other)))
            , peer(other?other->getPeer():Ptr<WeakPeer>(0))
    { }
    
    inline Ptr<T> lock() const {
        if (valid()) {
            return p;
        } else {
            return 0;
        }
    }
    inline WeakPtr<T> & operator= (T * obj) {
        p=obj;
        if(p) peer=obj->getPeer();
        return *this;
    }
    inline WeakPtr<T> & operator= (const Ptr<T> & ptr) {
        p=ptr.getPtr();
        if(p) peer=p->getPeer();
        return *this;
    };
    inline bool valid() const { return p!=0 && peer->object_alive; }
    inline operator bool() const { return valid(); }
    inline bool operator==(const WeakPtr<T> & ptr) const { return p == ptr.p; }
    inline bool operator!=(const WeakPtr<T> & ptr) const { return p != ptr.p; }
    inline bool operator<(const WeakPtr<T> & ptr) const { return p < ptr.p; }
    inline bool operator<=(const WeakPtr<T> & ptr) const { return p <= ptr.p; }
    inline bool operator>(const WeakPtr<T> & ptr) const { return p > ptr.p; }
    inline bool operator>=(const WeakPtr<T> & ptr) const { return p >= ptr.p; }
};


#endif
