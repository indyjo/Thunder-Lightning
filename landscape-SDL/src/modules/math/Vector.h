#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>
#include <cmath>
#include <cstdio>
#include <cassert>

#ifndef BOOST_STATIC_ASSERT
#ifdef WITHOUT_BOOST
    #define BOOST_STATIC_ASSERT(E) assert(E)
#else
    #include <boost/static_assert.hpp>
#endif
#endif

template<int N, class T> class XVector;

// Helper function used for interval vectors, where there is a better implementation
// for squaring
template<class T>
inline T square(const T & x) { return x*x; }

template<int N, class T>
class XVector {
protected:
    T v[N];

// Constructors
public:
    inline XVector() { }

    // convert from other XVector type
    template<class U>
    inline XVector(const XVector<N,U> & src) {
        for (int i=0; i<N; i++) v[i]=src[i];
    }

    // Copy an array of Ts*/
    inline XVector(const T * src) {
        for (int i=0; i<N; i++) v[i]=src[i];
    }

    inline XVector(T v0) {
        BOOST_STATIC_ASSERT(N==1);
        v[0] = v0;
    }
    inline XVector(T v0, T v1) {
        BOOST_STATIC_ASSERT(N==2);
        v[0] = v0;
        v[1] = v1;
    }
    inline XVector(T v0, T v1, T v2) {
        BOOST_STATIC_ASSERT(N==3);
        v[0] = v0;
        v[1] = v1;
        v[2] = v2;
    }
    inline XVector(T v0, T v1, T v2, T v3) {
        BOOST_STATIC_ASSERT(N==4);
        v[0] = v0;
        v[1] = v1;
        v[2] = v2;
        v[3] = v3;
    }
    inline XVector(T v0, T v1, T v2, T v3, T v4) {
        BOOST_STATIC_ASSERT(N==5);
        v[0] = v0;
        v[1] = v1;
        v[2] = v2;
        v[3] = v3;
        v[4] = v4;
    }

// Operators
    inline const XVector& operator= (const T * src) // Copy array
    {
        for (int i=0; i<N; i++)
            v[i]=src[i];
        return *this;
    }

    inline const T & operator[] (int i) const {        // const element access
        assert((0<=i) && (i<N));
        return v[i];
    }

    inline T & operator[] (int i) {                  // Element access
        assert(0<=i && i<N);
        return v[i];
    }

    inline friend XVector operator+ (const XVector& a,
                                     const XVector& b) {
        return XVector(a) += b;
    }
    inline friend XVector operator- (const XVector& a,
                                     const XVector& b) {
        return XVector(a) -= b;
    }
    // Scalar product
    inline friend T operator* (const XVector& a, const XVector& b) {
        T result=0;
        for (int i=0; i<N; i++) result += a[i] * b[i];
        return result;
    }


    // Scalar multiplication
    inline friend XVector operator* (const XVector& v, T lambda)
    { return XVector(v) *= lambda; }

    inline friend XVector operator/ (const XVector& v, T lambda)
    { return XVector(v) *= 1/lambda; }

    inline friend XVector operator* (T lambda, const XVector& v)
    { return XVector(v) *= lambda; }

    inline XVector& operator*= (T lambda) {
        for (int i=0; i<N; i++) v[i]*=lambda;
        return *this;
    }

    inline XVector& operator/= (T lambda) {
        T oolambda = 1/lambda;
        for (int i=0; i<N; i++) v[i]*=oolambda;
        return *this;
    }

    inline XVector& operator+= (const XVector& a) {
        for (int i=0; i<N; i++) v[i]+=a[i];
        return *this;
    }

    inline XVector& operator-= (const XVector& a) {
        for (int i=0; i<N; i++) v[i]-=a[i];
        return *this;
    }


    inline XVector& operator+ () { return *this; }
    inline const XVector& operator+ () const { return *this; }
    inline XVector operator- () const
    {
        XVector result;
        for (int i=0; i<N; i++) result.v[i]= - v[i];
        return result;
    }

// Length and normalization functions
    inline T length() const {
        return sqrt( lengthSquare() );
    }
    inline T lengthSquare() const {
        T result = 0;
        for(int i=0; i<N; i++) result += square(v[i]);
        return result;
    }
    inline XVector& normalize() {
        (*this) /= length();
        return *this;
    }

// Cross product for dim 3
    inline XVector operator% (const XVector & b) const {
        BOOST_STATIC_ASSERT(N==3);
        const XVector & a = *this;
        XVector r;
        r[0] = a[1]*b[2] - b[1]*a[2];
        r[1] = a[2]*b[0] - b[2]*a[0];
        r[2] = a[0]*b[1] - b[0]*a[1];
        return r;
    }

    inline const XVector & operator%= (const XVector & a) {
        BOOST_STATIC_ASSERT(N==3);
        return *this = (*this) % a;
    }

// Dumping to stdout for debugging purposes
    inline void dump() const {
        printf("[ ");
        for(int i=0; i<N; i++)
            printf("%6.2f%c", v[i], (i==N-1)?']':' ');
        printf("\n");
    }
};

template<int N, class T>
std::ostream & operator<< (std::ostream & os,
                           const XVector<N,T> & v);
template<int N, class T>
std::istream & operator>> (std::istream & is,
                           XVector<N,T> & v);

typedef XVector<3, float> Vector;
typedef XVector<2, float> Vector2;
typedef XVector<3, float> Vector3;
typedef XVector<4, float> Vector4;
typedef XVector<5, float> Vector5;
typedef XVector<6, float> Vector6;
typedef XVector<7, float> Vector7;
typedef XVector<8, float> Vector8;




#endif
