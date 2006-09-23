#ifndef SPECIAL_MATRICES_H
#define SPECIAL_MATRICES_H

#include "MatrixVector.h"

template<class T>
inline XMatrix<2, T> MatrixFromColumns(const XVector<2,T> & v0,
                                       const XVector<2,T> & v1)
{
    XMatrix<2,T> result;
    for(int i=0; i<2; i++) result(i,0) = v0[i];
    for(int i=0; i<2; i++) result(i,1) = v1[i];
    return result;
}

template<class T>
inline XMatrix<3, T> MatrixFromColumns(const XVector<3,T> & v0,
                                       const XVector<3,T> & v1,
                                       const XVector<3,T> & v2)
{
    XMatrix<3,T> result;
    for(int i=0; i<3; i++) result(i,0) = v0[i];
    for(int i=0; i<3; i++) result(i,1) = v1[i];
    for(int i=0; i<3; i++) result(i,2) = v2[i];
    return result;
}

template<class T>
inline XMatrix<4, T> MatrixFromColumns(const XVector<4,T> & v0,
                                       const XVector<4,T> & v1,
                                       const XVector<4,T> & v2,
                                       const XVector<4,T> & v3)
{
    XMatrix<4,T> result;
    for(int i=0; i<4; i++) result(i,0) = v0[i];
    for(int i=0; i<4; i++) result(i,1) = v1[i];
    for(int i=0; i<4; i++) result(i,2) = v2[i];
    for(int i=0; i<4; i++) result(i,2) = v3[i];
    return result;
}


template<class T>
inline XMatrix<2, T> MatrixFromRows(const XVector<2,T> & v0,
                                    const XVector<2,T> & v1)
{
    XMatrix<2,T> result;
    for(int i=0; i<2; i++) result(0,i) = v0[i];
    for(int i=0; i<2; i++) result(1,i) = v1[i];
    return result;
}

template<class T>
inline XMatrix<3, T> MatrixFromRows(const XVector<3,T> & v0,
                                    const XVector<3,T> & v1,
                                    const XVector<3,T> & v2)
{
    XMatrix<3,T> result;
    for(int i=0; i<3; i++) result(0,i) = v0[i];
    for(int i=0; i<3; i++) result(1,i) = v1[i];
    for(int i=0; i<3; i++) result(2,i) = v2[i];
    return result;
}

template<class T>
inline XMatrix<4, T> MatrixFromRows(const XVector<4,T> & v0,
                                    const XVector<4,T> & v1,
                                    const XVector<4,T> & v2,
                                    const XVector<4,T> & v3)
{
    XMatrix<4,T> result;
    for(int i=0; i<4; i++) result(0,i) = v0[i];
    for(int i=0; i<4; i++) result(1,i) = v1[i];
    for(int i=0; i<4; i++) result(2,i) = v2[i];
    for(int i=0; i<4; i++) result(3,i) = v3[i];
    return result;
}

template<int N, class T>
XMatrix<N,T> ZeroMatrix() {
    XMatrix<N,T> M;
    for(int i=0; i<N; i++) for(int j=0; j<N; j++)
        M(i,j) = 0;
    return M;
}

template<int N, class T>
inline XMatrix<N,T> IdentityMatrix() {
    XMatrix<N,T> M = ZeroMatrix<N,T>();
    for(int i=0; i<N; i++) M(i,i) = 1;
    return M;
}

template<int N, class T>
XMatrix<N,T> TranslateMatrix(const XVector<N-1,T> & x)
{
    XMatrix<N,T> M = IdentityMatrix<N,T>();
    for(int i=0; i<N-1; i++)
        M(i,N-1) = x[i];
    return M;
}

template<int N, class T>
XMatrix<N,T> ScaleMatrix(T s)
{
    XMatrix<N,T> M = ZeroMatrix<N,T>();
    for(int i=0; i<N; i++)
        M(i,i) = s;
    return M;
}

template<int N, class T>
XMatrix<N,T> ScaleMatrixHom(T s)
{
    XMatrix<N,T> M = ZeroMatrix<N,T>();
    for(int i=0; i<N-1; i++)
        M(i,i) = s;
    M(N-1, N-1) = 1;
    return M;
}

template<int N, class T>
XMatrix<N,T> RotateMatrix(int dim1, int dim2, T a)
{
    XMatrix<N,T> M = IdentityMatrix<N,T>();
    T ca=cos(a);
    T sa=sin(a);

    M(dim1,dim1) = ca;
    M(dim2,dim2) = ca;
    M(dim1,dim2) = -sa;
    M(dim2,dim1) = sa;
    return M;
}

template<class T>
XMatrix<3, T> RotateXMatrix(T a) { return RotateMatrix<3,T>(1,2,a); }
template<class T>
XMatrix<3, T> RotateYMatrix(T a) { return RotateMatrix<3,T>(2,0,a); }
template<class T>
XMatrix<3, T> RotateZMatrix(T a) { return RotateMatrix<3,T>(0,1,a); }


// Returns v * v^T
template<int N, class T>
XMatrix<N,T> ProjectionMatrix(const XVector<N,T> & v) {
    XMatrix<N,T> M;
    for(int i=0; i<N; i++) for (int j=0; j<N; j++)
        M(i,j) = v[i]*v[j];
    return M;
}

// Returns matrix M with M*v = a % v for all v
template<class T>
XMatrix<3,T> CrossProductMatrix(const XVector<3,T> & a)
{
    return MatrixFromColumns(XVector<3,T>(     0, -a[2],  a[1]),
                             XVector<3,T>(  a[2],     0, -a[0]),
                             XVector<3,T>( -a[1],  a[0],     0));
}

// Let v be a normalized vector, v=[x,y,z]
// Also let
//     | 0 -z  y |
// S = | z  0 -x |
//     |-y  x  0 |
//
// M = v*v^T + (cos a)*(I - v*v^T) + (sin a)*S
//
// Then the resulting XMatrix<N,T>
//     | m m m 0 |
// R = | m m m 0 |
//     | m m m 0 |
//     | 0 0 0 1 |
// where m represents elements from M, which is a 3x3 XMatrix<N,T>

template<class T>
XMatrix<3,T> RotateAxisMatrix(XVector<3,T> &v, T a)
{
    v.normalize();
    XMatrix<3,T> S = CrossProductMatrix(v);
    XMatrix<3,T> P = ProjectionMatrix<3>(v);
    return P + cos(a)*(IdentityMatrix<3,float>() - P) + sin(a)*S;
}


#endif
