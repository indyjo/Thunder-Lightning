#ifndef MATRIX_H
#define MATRIX_H

#include <ostream>
#include <cmath>
#include <cstdio>
#include <cassert>
#include "Vector.h"

#ifndef BOOST_STATIC_ASSERT
#ifdef WITHOUT_BOOST
    #define BOOST_STATIC_ASSERT(E) assert(E)
#else
    #include <boost/static_assert.hpp>
#endif
#endif

/*

We define a matrix as an array of N row vectors each with N entries. So we get
an element order just like in OpenGL:

|  1   5   9  13  |
|  2   6  10  14  |
|  3   7  11  15  |
|  4   8  12  16  |

*/


template<int N, class T> class XMatrix
{
protected:
    T m[N*N];

// Constructors
public:
    inline XMatrix() {}

    inline XMatrix(T m00, T m01,
                   T m10, T m11)
    {
        BOOST_STATIC_ASSERT(N==2);
        XMatrix & m=*this;
        m(0,0) = m00;
        m(0,1) = m01;
        m(1,0) = m10;
        m(1,1) = m11;
    }

    inline XMatrix(T m00, T m01, T m02,
                   T m10, T m11, T m12,
                   T m20, T m21, T m22)
    {
        BOOST_STATIC_ASSERT(N==3);
        XMatrix & m=*this;
        m(0,0) = m00;
        m(0,1) = m01;
        m(0,2) = m02;
        m(1,0) = m10;
        m(1,1) = m11;
        m(1,2) = m12;
        m(2,0) = m20;
        m(2,1) = m21;
        m(2,2) = m22;
    }

    inline XMatrix(T m00, T m01, T m02, T m03,
                   T m10, T m11, T m12, T m13,
                   T m20, T m21, T m22, T m23,
                   T m30, T m31, T m32, T m33)
    {
        BOOST_STATIC_ASSERT(N==4);
        XMatrix & m=*this;
        m(0,0) = m00;
        m(0,1) = m01;
        m(0,2) = m02;
        m(0,3) = m03;
        m(1,0) = m10;
        m(1,1) = m11;
        m(1,2) = m12;
        m(1,3) = m13;
        m(2,0) = m20;
        m(2,1) = m21;
        m(2,2) = m22;
        m(2,3) = m23;
        m(3,0) = m30;
        m(3,1) = m31;
        m(3,2) = m32;
        m(3,3) = m33;
    }


    static XMatrix Array(const T * src);
    static XMatrix Rows(const XVector<N, T> * rows);
    static XMatrix Cols(const XVector<N, T> * cols);
    static XMatrix Hom(const XMatrix<N-1,T> & R, const XVector<N-1,T> & x);
    static XMatrix Hom(const XMatrix<N-1,T> & R);

// data access
    inline const T * raw() const { return m; }
    inline XVector<N,T> column(int n)
    { return XVector<N,T>(&m[N*n]); }
    inline XVector<N,T> row(int n)
    {
    	XVector<N,T> v;
    	for(int i=0;i<N;++i) v[i] = (*this)(n,i);
    	return v;
    }
    

// Operators
    // Element access via index operator []
    inline T operator[] (int i) const {
        assert((0<=i) && (i<=N*N));
        return m[i];
    }

    inline T & operator[] (int i) { // Non-const version
        assert((0<=i) && (i<=N*N));
        return m[i];
    }

    // Element access via M(i,j)
    inline T operator() (int i, int j) const {
        assert(i<N && j<N);
        return m[j*N+i];
    }

    inline T & operator() (int i, int j) { // Non-const version
        assert(i<N && j<N);
        return m[j*N+i];
    }

    inline XMatrix& operator += (const XMatrix &M) {
        for(int i=0; i<N*N; i++) m[i] += M[i];
        return *this;
    }

    inline XMatrix& operator -= (const XMatrix &M) {
        for(int i=0; i<N*N; i++) m[i] -= M[i];
        return *this;
    }

    inline XMatrix operator+ (const XMatrix &m) const {
        return XMatrix(*this) += m;
    }
    inline XMatrix operator- (const XMatrix &m) const {
        return XMatrix(*this) -= m;
    }

    inline const XMatrix & operator*= (const T & s) {
        for(int i=0; i<N*N; i++) m[i] *= s;
        return *this;
    }
    inline const XMatrix & operator/= (const T & s) {
        return (*this) *= 1/s;
    }

    inline XMatrix operator* (const T & s) const {
        return XMatrix(*this) *= s;
    }
    inline XMatrix operator/ (const T & s) const {
        return XMatrix(*this) /= s;
    }
    inline friend XMatrix operator* (const T & s, const XMatrix & M) {
        return M * s;
    }

    inline const XMatrix & operator+ () const { return *this; }
    inline XMatrix operator- () const {
        XMatrix M;
        for(int i=0; i<N*N; i++) M[i] = -m[i];
        return M;
    }

    //friend XMatrix operator* (const XMatrix &, const XMatrix &);
    inline friend XMatrix operator* (const XMatrix & A,
                                         const XMatrix & B) {
        T entry;
        XMatrix<N,T> R;

        for (int j=0; j<N; j++) for (int i=0; i<N; i++) {
            entry=0.0;
            for (int k=0; k<N; k++) {
                entry += A(i,k) * B(k,j);
            }
            R(i,j)=entry;
        }
        return R;
    }


// Functions
    XMatrix & transpose();
    XMatrix & transposeHom();

    inline friend std::ostream & operator<< (std::ostream & os, XMatrix M) {
        for(int i=0; i<N; i++) {
            os << "( ";
            for(int j=0; j<N; j++) {
                std::os << M(i,j);
                if (j==N-1) os << ")" << std::endl;
                else os << ", ";
            }
            os << std::endl;
        }
        return os;
    }


    void orthoNormalize();
    inline const T * glMatrix() const { return m; }
    inline T * glMatrix() { return m; }
    void dump() const;


};

template<int N, class T>
XMatrix<N,T> XMatrix<N,T>::Array(const T * src) {
    XMatrix<N,T> M;
    for(int i=0; i<N*N; i++)
        M.m[i] = src[i];
    return M;
}

template<int N, class T>
XMatrix<N,T> XMatrix<N,T>::Rows(const XVector<N,T> * rows) {
    XMatrix<N,T> M;
    for(int i=0; i<N; i++) for(int j=0; j<N; j++)
        M(i,j) = rows[i][j];
    return M;
}

template<int N, class T>
XMatrix<N,T> XMatrix<N,T>::Cols(const XVector<N,T> * cols) {
    XMatrix<N,T> M;
    for(int i=0; i<N; i++) for(int j=0; j<N; j++)
        M(i,j) = cols[j][i];
    return M;
}

template<int N, class T>
XMatrix<N,T> XMatrix<N,T>::Hom(const XMatrix<N-1,T> & R,
                                      const XVector<N-1,T> & x) {
    XMatrix<N,T> M;
    for(int i=0; i<N-1; i++) for (int j=0; j<N-1; j++)
        M(i,j) = R(i,j);
    for(int i=0; i<N-1; i++) {
        M(i,N-1) = x[i];
        M(N-1,i) = 0;
    }
    M(N-1, N-1) = 1;
    return M;
}

template<int N, class T>
XMatrix<N,T> XMatrix<N,T>::Hom(const XMatrix<N-1,T> & R) {
    XMatrix<N,T> M;
    for(int i=0; i<N-1; i++) for (int j=0; j<N-1; j++)
        M(i,j) = R(i,j);
    for(int i=0; i<N-1; i++) {
        M(i,N-1) = 0;
        M(N-1, i) = 0;
    }
    M(N-1, N-1) = 1;
    return M;
}

// Matrix multiplication
template<int N, class T>
XMatrix<N,T> operator* (const XMatrix<N,T> &A, const XMatrix<N,T> &B)
{
    T entry;
    XMatrix<N,T> R;

    for (int j=0; j<N; j++) for (int i=0; i<N; i++) {
        entry=0.0;
        for (int k=0; k<N; k++) {
            entry += A(i,k) * B(k,j);
        }
        R(i,j)=entry;
    }
    return R;
}

template<int N, class T>
XMatrix<N,T>& XMatrix<N,T>::transpose() {
    for(int r=0; r<N; r++) for(int c=r+1; c<N; c++)
        std::swap((*this)(c,r), (*this)(r,c));
    return *this;
}

template<int N, class T>
XMatrix<N,T>& XMatrix<N,T>::transposeHom() {
    for(int r=0; r<N-1; r++) for(int c=r+1; c<N-1; c++)
        std::swap((*this)(c,r), (*this)(r,c));
    return *this;
}

template<int N, class T>
void XMatrix<N,T>::orthoNormalize() {
    BOOST_STATIC_ASSERT(N==4 || N==3);

    if (N==4) {
        XMatrix<4,T> & M = (XMatrix<4,T> &) *this;
        XVector<3,T> e1(M(0,0), M(1,0), M(2,0));
        XVector<3,T> e2(M(0,1), M(1,1), M(2,1));
        XVector<3,T> e3(e1.normalize() % e2.normalize());
        e3.normalize();

        M(0,0) = e1[0]; M(0,1) = e2[0]; M(0,2) = e3[0];
        M(1,0) = e1[1]; M(1,1) = e2[1]; M(1,2) = e3[1];
        M(2,0) = e1[2]; M(2,1) = e2[2]; M(2,2) = e3[2];
    } else {
        XMatrix<3,T> & M = (XMatrix<3,T> &) *this;
        XVector<3,T> e1(M(0,0), M(1,0), M(2,0));
        XVector<3,T> e2(M(0,1), M(1,1), M(2,1));
        XVector<3,T> e3(e1.normalize() % e2.normalize());
        e3.normalize();

        M(0,0) = e1[0]; M(0,1) = e2[0]; M(0,2) = e3[0];
        M(1,0) = e1[1]; M(1,1) = e2[1]; M(1,2) = e3[1];
        M(2,0) = e1[2]; M(2,1) = e2[2]; M(2,2) = e3[2];
    }
}

template<int N, class T>
void XMatrix<N,T>::dump() const {
    for(int r=0; r<N; r++) {
        printf("| ");
        for(int c=0; c<N; c++) printf("%3.2f ", (*this)(r,c));
        printf("|\n");
    }
}

typedef XMatrix<4, float> Matrix;
typedef XMatrix<2, float> Matrix2;
typedef XMatrix<3, float> Matrix3;
typedef XMatrix<4, float> Matrix4;
typedef XMatrix<5, float> Matrix5;
typedef XMatrix<6, float> Matrix6;
typedef XMatrix<7, float> Matrix7;
typedef XMatrix<8, float> Matrix8;

#endif
