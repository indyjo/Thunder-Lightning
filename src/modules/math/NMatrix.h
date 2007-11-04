#ifndef NMATRIX_H
#define NMATRIX_H

#include <ostream>
#include <cmath>
#include <cstdio>
#include <cassert>
#include <debug.h>

#define ls_message(...) 

/*

An NMatrix stores a variable-sized n*m matrix in column-major order (like
OpenGL):
E.g., a 4*3 matrix is stored in the following way:
|  0   4   8  |
|  1   5   9  |
|  2   6  10  |
|  3   7  11  |

*/


template<class T> class NMatrix
{
    NMatrix();
protected:
    int rows, cols;
    T *m;

    void freeData() { ls_message("Deleting mat: %p\n", m); delete[] m; }
    void allocData() { ls_message("Beginning alloc (%dx%d).\n", rows, cols); m = new T[rows*cols]; ls_message("New mat: %p\n", m); }
// Constructors
public:
    inline NMatrix(int r, int c) : rows(r), cols(c)
    { allocData(); }
    inline NMatrix(int r, int c, const T& init) : rows(r), cols(c)
    { allocData(); setTo(init); }
    inline NMatrix(int r, int c, const T* init) : rows(r), cols(c) {
        allocData();
        for(int i=0; i<r*c; ++i) m[i] = init[i];
    }
    
    inline ~NMatrix() { freeData(); }
    inline NMatrix(const NMatrix<T> & other) {
        ls_message("NMatrix: %p initialized from %p (%p)\n", this, &other, other.m);
        rows = other.rows;
        cols = other.cols;
        allocData();
        for(int i=0; i<rows*cols; ++i) m[i] = other.m[i];
        ls_message(" ...initialized\n");
    }
    
    const NMatrix& operator= (const NMatrix & other) {
        ls_message("Matrox %p <- %p (%p)\n", this, &other, other.m);
        if (rows!=other.rows || cols != other.cols) {
            redim(other.rows, other.cols);
        }
        for(int i=0; i<rows*cols; ++i) m[i] = other.m[i];
        return *this;
    }
    
    void redim(int r, int c) {
        ls_message("Redim to %dx%d\n", r,c);
        freeData();
        rows = r;
        cols = c;
        allocData();
    }
    
    inline void setTo(const T& val) {
        for(int i=0; i<rows*cols; ++i) m[i] = val;
    }
    inline void clear() { setTo(0); }

// data access
    inline const T* rawData() const { return m; }
    inline T* rawDataForWriting() { return m; }
    
    inline int nrows() const { return rows; }
    inline int ncols() const { return cols; }
    
// Operators
    // Element access via index operator []
    inline T operator[] (int i) const {
        assert((0<=i) && (i<cols*rows));
        return m[i];
    }

    inline T & operator[] (int i) { // Non-const version
        assert((0<=i) && (i<cols*rows));
        return m[i];
    }

    // Element access via M(i,j)
    inline T operator() (int i, int j) const {
        assert(i<rows && j<cols && i>=0 && j>= 0);
        return m[j*rows+i];
    }

    inline T& operator() (int i, int j) { // Non-const version
        assert(i<rows && j<cols && i>=0 && j>= 0);
        return m[j*rows+i];
    }

    inline NMatrix& operator += (const NMatrix &M) {
        assert(M.rows == rows && M.cols == cols);
        for(int i=0; i<rows*cols; i++) m[i] += M[i];
        return *this;
    }

    inline NMatrix& operator -= (const NMatrix &M) {
        assert(M.rows == rows && M.cols == cols);
        for(int i=0; i<rows*cols; i++) m[i] -= M[i];
        return *this;
    }

    inline NMatrix operator+ (const NMatrix &m) const {
        return NMatrix(*this) += m;
    }
    inline NMatrix operator- (const NMatrix &m) const {
        return NMatrix(*this) -= m;
    }

    inline const NMatrix & operator*= (const T & s) {
        for(int i=0; i<rows*cols; i++) m[i] *= s;
        return *this;
    }
    inline const NMatrix & operator/= (const T & s) {
        return (*this) *= 1/s;
    }

    inline NMatrix operator* (const T & s) const {
        return NMatrix(*this) *= s;
    }
    inline NMatrix operator/ (const T & s) const {
        return NMatrix(*this) /= s;
    }
    inline friend NMatrix operator* (const T & s, const NMatrix & M) {
        return M * s;
    }

    inline const NMatrix & operator+ () const { return *this; }
    inline NMatrix operator- () const {
        NMatrix M;
        for(int i=0; i<rows*cols; i++) M[i] = -m[i];
        return M;
    }

    friend NMatrix operator* (const NMatrix & A,
                              const NMatrix & B) {
        assert(A.cols == B.rows);
        
        NMatrix R(A.rows, B.cols);
        for (int j=0; j<B.cols; j++) for (int i=0; i<A.rows; i++) {
            T entry=0;
            for (int k=0; k<A.cols; k++) {
                entry += A(i,k) * B(k,j);
            }
            R(i,j)=entry;
        }
        return R;
    }

    friend NMatrix operator% (const NMatrix & A,
                              const NMatrix & B) {
        assert(A.cols * A.rows == 3);
        assert(B.cols * B.rows == 3);
        
        T values[] = {
            A[1] * B[2] - A[2] * A[1],
            A[2] * B[0] - A[0] * B[2],
            A[0] * B[1] - A[1] * B[0]
        };
        
        return NMatrix(3, 1, values);
    }
    
    friend NMatrix transposed(const NMatrix & M) {
        NMatrix result(M.cols, M.rows);
        for (int i=0; i<M.rows; ++i) for (int j=0; j<M.cols; ++j) {
            result(j,i) = M(i,j);
        }
        return result;
    }

    void transpose() {
        if( rows <=1 || cols <= 1) {
            std::swap(rows,cols);
        } else if ( rows == cols ) {
            for(int r=0; r<rows; r++) for(int c=r+1; c<cols; c++)
                std::swap((*this)(c,r), (*this)(r,c));
        } else {
            *this = transposed(*this);
        }
    }

    void dump() const {
        for(int r=0; r<rows; r++) {
            printf("| ");
            for(int c=0; c<cols; c++) printf("%3.2f ", (*this)(r,c));
            printf("|\n");
        }
    }
    
    T inner(const NMatrix & other) const {
        assert(rows*cols == other.rows*other.cols);
        T result = 0;
        for (int i=0; i<rows*cols; ++i) result += m[i]*other.m[i];
        return result;
    }
};

#endif
