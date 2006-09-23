#ifndef QUATERNION_H
#define QUATERNION_H

// For an introduction on quaternions see:
// http://www.cs.berkeley.edu/~laura/cs184/quat/quaternion.html

#include <cmath>
#include "Vector.h"
#include "Matrix.h"

template<class T> class XQuaternion;
/*
template<class T> XQuaternion<T> operator/ (const XQuaternion<T> & q, T s);
template<class T> XQuaternion<T> operator* (const XQuaternion<T> &q1, const XQuaternion<T> & q2);
*/
typedef XQuaternion<float> Quaternion;

template<class T>
class XQuaternion {
    T u;
    XVector<3,T> v;

public:
    inline XQuaternion() { }
    inline XQuaternion(T w, T x, T y, T z) : u(w), v(x,y,z) { }
    inline XQuaternion(T u, const XVector<3,T> v) : u(u), v(v) { }
    inline XQuaternion(const T *f) : u(f[0]), v(f[1], f[2], f[3]) { }
    template<class S> inline XQuaternion<T>(const XQuaternion<S> & q) : u(q.real()), v(q.imag()) { }
    template<class S> inline const XQuaternion<T> & operator= (const XQuaternion<S> & q) {
        u = q.real();
        v = q.imag();
        return *this;
    }

    // Constructs a XQuaternion from a rotation around an axis
    // The axis vector has to be unit length
    inline static XQuaternion Rotation(const XVector<3,T> & axis, T angle) {
        XQuaternion q;
        angle/=2;
        q.u = cos(angle);
        q.v = axis * sin(angle);
        return q;
    }

    inline const T & real() const { return u; }
    inline const XVector<3,T> & imag() const { return v; }
    inline T  & real() { return u; }
    inline XVector<3,T> & imag() { return v; }

    inline friend T  real(const XQuaternion & q) { return q.real(); }
    inline friend XVector<3,T> imag(const XQuaternion & q) { return q.imag(); }

    inline XQuaternion conj() const { return XQuaternion(u, -v); }
    inline XQuaternion inv() const { return conj() / normSquare(); }
    inline T normSquare() const { return square(u) + v.lengthSquare();}
    inline T norm() const { return sqrt(normSquare()); }
    inline XQuaternion normalize() const { return (*this) / norm(); }

    inline XVector<3,T> rot(const XVector<3,T> & v) const
    { return ((*this) * XQuaternion(0, v) * conj()).imag(); }

    inline void toMatrix(XMatrix<4,T> & m) const {
    	/*
        T ww = u*u;
        T xx = v[0]*v[0];
        T yy = v[1]*v[1];
        T zz = v[2]*v[2];
        T w2 = 2*u;
        T wx2 = w2*v[0];
        T wy2 = w2*v[1];
        T wz2 = w2*v[2];
        T xy2 = 2*v[0]*v[1];
        T xz2 = 2*v[0]*v[2];
        T yz2 = 2*v[1]*v[2];

        m(0,0) = ww + xx - yy - zz;
        m(1,0) = xy2 - wz2;
        m(2,0) = xz2 + wy2;
        m(3,0) = 0;

        m(0,1) = xy2 + wz2;
        m(1,1) = ww - xx + yy - zz;
        m(2,1) = yz2 - wx2;
        m(3,1) = 0;

        m(0,2) = xz2 - wy2;
        m(1,2) = yz2 + wx2;
        m(2,2) = ww - xx - yy + zz;
        m(3,2) = 0;

        m(0,3) = 0;
        m(1,3) = 0;
        m(2,3) = 0;
        m(3,3) = ww + xx + yy + zz;*/
        
        m = XMatrix<4,T>::Hom(MatrixFromColumns(
        	rot(Vector(1,0,0)),
        	rot(Vector(0,1,0)),
        	rot(Vector(0,0,1))));
    }

    inline void toMatrix(XMatrix<3,T> & m) const {
    	/*
        T ww = u*u;
        T xx = v[0]*v[0];
        T yy = v[1]*v[1];
        T zz = v[2]*v[2];
        T w2 = 2*u;
        T wx2 = w2*v[0];
        T wy2 = w2*v[1];
        T wz2 = w2*v[2];
        T xy2 = 2*v[0]*v[1];
        T xz2 = 2*v[0]*v[2];
        T yz2 = 2*v[1]*v[2];

        m(0,0) = ww + xx - yy - zz;
        m(1,0) = xy2 - wz2;
        m(2,0) = xz2 + wy2;

        m(0,1) = xy2 + wz2;
        m(1,1) = ww - xx + yy - zz;
        m(2,1) = yz2 - wx2;

        m(0,2) = xz2 - wy2;
        m(1,2) = yz2 + wx2;
        m(2,2) = ww - xx - yy + zz;*/
        m = MatrixFromColumns(
        	rot(Vector(1,0,0)),
        	rot(Vector(0,1,0)),
        	rot(Vector(0,0,1)));
    }

    inline void fromMatrix(const XMatrix<3,T> & M) {
        T trace = M(0,0)+M(1,1)+M(2,2)+1;
        T s;

        //ls_message("trace: %f\n", trace);
        if (trace > 0.001) {
            s = 0.5 / sqrt(trace);
            u = 0.25 / s;
            v[0] = (M(2,1)-M(1,2)) * s;
            v[1] = (M(0,2)-M(2,0)) * s;
            v[2] = (M(1,0)-M(0,1)) * s;
            return;
        }
        int c = 0;
        if (M(1,1) > M(0,0)) c = 1;
        if (M(2,2) > M(c,c)) c = 2;
        //ls_message("Major diagonal element: %d (%f)\n", c, M(c,c));
        //M.dump();
        switch(c) {
        case 0:
            s = 2 * sqrt( 1.0 + M(0,0) - M(1,1) - M(2,2));
            v[0] = -0.25 * s;
            v[1] = -(M(0,1) + M(1,0)) / s;
            v[2] = -(M(0,2) + M(2,0)) / s;
            u = (M(1,2) - M(2,1)) / s;
            return;
        case 1:
            s = 2 * sqrt( 1.0 + M(1,1) - M(0,0) - M(2,2));
            v[0] = (M(0,1) + M(1,0)) / s;
            v[1] = 0.25 * s;
            v[2] = (M(1,2) + M(2,1)) / s;
            u = (M(0,2) - M(2,0)) / s;
            return;
        default:
            s = 2 * sqrt( 1.0 + M(2,2) - M(0,0) - M(1,1));
            v[0] = -(M(0,2) + M(2,0)) / s;
            v[1] = -(M(1,2) + M(2,1)) / s;
            v[2] = -0.25 * s;
            u = (M(0,1) - M(1,0)) / s;
            return;
        }
    }
};

template<class T>
inline XQuaternion<T> operator+ (const XQuaternion<T> & q1, const XQuaternion<T> & q2) {
    return XQuaternion<T>(q1.real()+q2.real(), q1.imag()+q2.imag());
}

template<class T>
inline XQuaternion<T> operator- (const XQuaternion<T> & q1, const XQuaternion<T> & q2) {
    return XQuaternion<T>(q1.real()-q2.real(), q1.imag()-q2.imag());
}

template<class T>
inline XQuaternion<T> & operator+= (XQuaternion<T> & q1, const XQuaternion<T> & q2) {
    q1.real() += q2.real();
    q1.imag() += q2.imag();
    return q1;
}

template<class T>
inline XQuaternion<T> & operator-= (XQuaternion<T> & q1, const XQuaternion<T> & q2) {
    q1.real() -= q2.real();
    q1.imag() -= q2.imag();
    return q1;
}

template<class T>
inline XQuaternion<T> operator* (const XQuaternion<T> & q, const T & s) {
    return XQuaternion<T>(s*q.real(), s*q.imag());
}

template<class T>
inline XQuaternion<T> operator* (const T & s, const XQuaternion<T> & q) {
    return XQuaternion<T>(s*q.real(), s*q.imag());
}

template<class T>
inline XQuaternion<T> operator/ (const XQuaternion<T> & q, const T & s) {
    T s_inv = 1/s;
    return XQuaternion<T>(s_inv*q.real(), s_inv*q.imag());
}

template<class T>
inline XQuaternion<T> & operator*= (XQuaternion<T> & q, const T & s) {
    q.real() *= s;
    q.imag() *= s;
    return q;
}

template<class T>
inline XQuaternion<T> & operator/= (XQuaternion<T> & q, const T & s) {
    T s_inv = 1/s;
    q.real() *= s_inv;
    q.imag() *= s_inv;
    return q;
}

template<class T>
inline XQuaternion<T> operator* (const XQuaternion<T> &q1, const XQuaternion<T> & q2) {
    return XQuaternion<T>(
            q1.real()*q2.real() - q1.imag()*q2.imag(),
            q1.real()*q2.imag() + q2.real()*q1.imag() + q1.imag() % q2.imag());
}

template<class T>
inline XQuaternion<T> operator- (const XQuaternion<T> &q) {
    return XQuaternion<T>(-q.real(), -q.imag());
}

#endif
