#ifndef _JOGI_MATRIX_H
#define _JOGI_MATRIX_H

#include "types3d.h"

class JMatrix
{
public:
    jmatrix_t matrix;

    /* Does nothing*/
    JMatrix();

    /* Inits the matrix with the given matrix struct */
    JMatrix(const jmatrix_t *m);

    /* Inits to the identity matrix */
    void init();

    /* Inits to the given matrix struct */
    void init(const jmatrix_t *m);

    /* Inits to a translate transformation matrix */
    void initTranslate(float x, float y, float z);

    /* Inits to a scale transformation matrix */
    void initScale(float sx, float sy, float sz);

    /* Inits to a rotate around X axis matrix */
    void initRotateX(float a);

    /* Inits to a rotate around Y axis matrix */
    void initRotateY(float a);

    /* Inits to a rotate around Z axis matrix */
    void initRotateZ(float a);

    /* Inits the matrix to B x A */
    void initMultiplyBA(const jmatrix_t *A, const jmatrix_t *B);
    
    /* Adds a matrix */
    void add(const jmatrix_t *m);

    /* Subtracts a matrix */
    void sub(const jmatrix_t *m);

    /* Translates the matrix */
    void translate(float x, float y, float z);

    /* Translates the matrix */
    void translate(const jpoint_t *p);

    /* scales the matrix */
    void scale(float s);

    /* scales the matrix at a point */
    void scale(float s, const jpoint_t *p);

    /* Dumps the matrix values to stderr */
    void dump();
};

#endif
