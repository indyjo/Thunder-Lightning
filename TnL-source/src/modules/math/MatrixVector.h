#ifndef MATRIX_VECTOR_H
#define MATRIX_VECTOR_H

#include "Matrix.h"
#include "Vector.h"

// matrix multiplication
// Instead of v = M * v
// you can now write
// v *= M
template<int N, class T>
inline XVector<N,T>& operator*= (XVector<N,T> & v, const XMatrix<N, T> & A) {
    int r,c;
    T result[N];
    for (r=0; r<N; r++) {
        result[r]=0;
        for (c=0; c<N; c++) result[r]+= v[c] * A(r,c);
    }
    v = result;
    return v;
}

// homogenous matrix multiplication
template<int N, class T>
inline XVector<N,T>& operator*= (XVector<N,T> & v, const XMatrix<N+1, T> & A) {
    int r,c;
    T result[N];
    for (r=0; r<N; r++) {
        result[r]=0;
        for (c=0; c<N; c++) result[r]+= v[c] * A(r,c);
        result[r]+=A(r, N);
    }
    v = result;
    return v;
}

// Matrix product


template<int N, int M, class T>
inline XVector<N,T> operator* (const XMatrix<M,T> & A,
                               const XVector<N,T> & a)
{
    BOOST_STATIC_ASSERT(N == M || M == (N+1));
    XVector<N,T> v(a);
    return operator*=<N,T>(v,A);
}


#endif
