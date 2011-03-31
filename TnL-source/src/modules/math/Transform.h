#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Vector.h"
#include "Quaternion.h"
#include "SpecialMatrices.h"

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

    static inline XTransform<T> identity() { return XTransform<T>(Quat(1,0,0,0),Vec(0,0,0)); }

    inline const Quat & quat() const { return q; }
    inline const Vec & vec() const { return t; }

    inline Quat & quat() { return q; }
    inline Vec & vec() { return t; }

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
    
    inline XTransform inv() const {
        return XTransform(q.conj(), -q.conj().rot(t));
    }
    
    inline Matrix toMatrix() const {
        Matrix mat;
        q.toMatrix(mat);
        mat(0,3) = t[0];
        mat(1,3) = t[1];
        mat(2,3) = t[2];
        return mat;
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
