#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Vector.h"
#include "Quaternion.h"

template<class T>
class XTransform {
    typedef XQuaternion<T> Quat;
    typedef XVector<3,T> Vec;
    Quat q;  // rotation
    Vec  t;  // translation

public:
    inline XTransform() { }
    inline XTransform(const Quat & q, const Vec & t)
    : q(q), t(t)
    { }

    inline const Quat & quat() const { return q; }
    inline const Vec & vec() const { return t; }

    inline XTransform operator* (const XTransform & trans) const {
        return XTransform(q*trans.q, q.rot(trans.t) + t);
    }

    inline XTransform & operator*= (const XTransform & trans) {
        *this = (*this) * trans;
    }

    inline Vec operator() (const Vec & v) const {
        return q.rot(v) + t;
    }

    inline XTransform & normalize() {
        q = q.normalize();
        return *this;
    }
    
    inline XTransform inv() {
        return XTransform(q.conj(), -q.conj().rot(t));
    }

    /*
    inline friend XTransform interp(T u, const XTransform & T1, const XTransform & T2) {
        return XTransform(
            (1-u) * T1.q + u * T2.q,
            (1-u) * T1.t + u * T2.t);
    }*/
    inline friend XTransform interp(T u, const XTransform & T1, const XTransform & T2) {
        return XTransform(
            T1.q + u * (T2.q-T1.q),
            T1.t + u * (T2.t-T1.t));
    }

    /*
    template<class U>
    inline friend XTransform<U> interp(U u, const XTransform & T1, const XTransform & T2) {
        return XTransform<U>(
            (1-u) * T1.q + u * T2.q,
            (1-u) * T1.t + u * T2.t);
    }
    */
    
    template<class U>
    inline operator XTransform<U> () const {
        return XTransform<U>((XQuaternion<U>) q, (XVector<3,U>) t);
    }
};


typedef XTransform<float> Transform;

#endif
