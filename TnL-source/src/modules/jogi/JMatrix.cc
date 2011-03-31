#include <math.h>
#include <iostream>
#include "jogi.h"


using namespace std;

/* Konstruktoren -----------------------------------------------------*/
JMatrix::JMatrix()
{
}

JMatrix::JMatrix(const jmatrix_t *m)
{
    matrix=*m;
}

/* Initialisierungsfunktionen ---------------------------------------------*/

void JMatrix::init()
{
	matrix.m[0][0]=1.0f;
	matrix.m[0][1]=0.0f;
	matrix.m[0][2]=0.0f;
	matrix.m[0][3]=0.0f;
	matrix.m[1][0]=0.0f;
	matrix.m[1][1]=1.0f;
	matrix.m[1][2]=0.0f;
	matrix.m[1][3]=0.0f;
	matrix.m[2][0]=0.0f;
	matrix.m[2][1]=0.0f;
	matrix.m[2][2]=1.0f;
	matrix.m[2][3]=0.0f;
	matrix.m[3][0]=0.0f;
	matrix.m[3][1]=0.0f;
	matrix.m[3][2]=0.0f;
	matrix.m[3][3]=1.0f;
}

void JMatrix::init(const jmatrix_t *m)
{
    matrix=*m;
}

void JMatrix::initRotateX(float a)
{
	float ca=cos(a);
    float sa=sin(a);
    
    matrix.m[0][0]=1;
    matrix.m[0][1]=0;
    matrix.m[0][2]=0;
    matrix.m[0][3]=0;
    matrix.m[1][0]=0;
    matrix.m[1][1]=ca;
	matrix.m[1][2]=-sa;
    matrix.m[1][3]=0;
    matrix.m[2][0]=0;
	matrix.m[2][1]=sa;
	matrix.m[2][2]=ca;
    matrix.m[2][3]=0;
	matrix.m[3][0]=0.0f;
	matrix.m[3][1]=0.0f;
	matrix.m[3][2]=0.0f;
    matrix.m[3][3]=1.0f;
}

void JMatrix::initRotateY(float a)
{
	float ca=cos(a);
    float sa=sin(a);
    
    matrix.m[0][0]=ca;
    matrix.m[0][1]=0;
    matrix.m[0][2]=sa;
    matrix.m[0][3]=0;
    matrix.m[1][0]=0;
    matrix.m[1][1]=1;
    matrix.m[1][2]=0;
    matrix.m[1][3]=0;
    matrix.m[2][0]=-sa;
    matrix.m[2][1]=0;
    matrix.m[2][2]=ca;
    matrix.m[2][3]=0;
	matrix.m[3][0]=0.0f;
	matrix.m[3][1]=0.0f;
	matrix.m[3][2]=0.0f;
    matrix.m[3][3]=1.0f;
}

void JMatrix::initRotateZ(float a)
{
	float ca=cos(a);
    float sa=sin(a);
    
	matrix.m[0][0]=ca;
    matrix.m[0][1]=-sa;
    matrix.m[0][2]=0;
    matrix.m[0][3]=0;
    matrix.m[1][0]=sa;
    matrix.m[1][1]=ca;
    matrix.m[1][2]=0;
    matrix.m[1][3]=0;
    matrix.m[2][0]=0;
    matrix.m[2][1]=0;
    matrix.m[2][2]=1;
    matrix.m[2][3]=0;
	matrix.m[3][0]=0.0f;
	matrix.m[3][1]=0.0f;
	matrix.m[3][2]=0.0f;
    matrix.m[3][3]=1.0f;
}

void JMatrix::initTranslate(float x, float y, float z)
{
	matrix.m[0][0]=1.0f;
	matrix.m[0][1]=0.0f;
	matrix.m[0][2]=0.0f;
	matrix.m[0][3]=x;
	matrix.m[1][0]=0.0f;
	matrix.m[1][1]=1.0f;
	matrix.m[1][2]=0.0f;
	matrix.m[1][3]=y;
	matrix.m[2][0]=0.0f;
	matrix.m[2][1]=0.0f;
	matrix.m[2][2]=1.0f;
	matrix.m[2][3]=z;
	matrix.m[3][0]=0.0f;
	matrix.m[3][1]=0.0f;
	matrix.m[3][2]=0.0f;
	matrix.m[3][3]=1.0f;
}

void JMatrix::initScale(float sx, float sy, float sz)
{
	matrix.m[0][0]=sx;
	matrix.m[0][1]=0.0f;
	matrix.m[0][2]=0.0f;
	matrix.m[0][3]=0.0f;
	matrix.m[1][0]=0.0f;
	matrix.m[1][1]=sy;
	matrix.m[1][2]=0.0f;
	matrix.m[1][3]=0.0f;
	matrix.m[2][0]=0.0f;
	matrix.m[2][1]=0.0f;
	matrix.m[2][2]=sz;
	matrix.m[2][3]=0.0f;
	matrix.m[3][0]=0.0f;
	matrix.m[3][1]=0.0f;
	matrix.m[3][2]=0.0f;
    matrix.m[3][3]=1.0f;
}


void JMatrix::initMultiplyBA(const jmatrix_t *A, const jmatrix_t *B)
{
    int i,j,k;

    for (i=0;i<4;i++) {
        for (j=0;j<4;j++) {
            matrix.m[i][j]=0;
            for (k=0;k<4;k++) {
                matrix.m[i][j]+=B->m[i][k]*A->m[k][j];
            }
        }
    }
}


/* Operatoren -------------------------------------------------------------*/

void JMatrix::add(const jmatrix_t *m)
{
    int i,j;

    for (j=0;j<4;j++) {
        for (i=0;i<4;i++) {
            matrix.m[i][j]+=m->m[i][j];
        }
    }
}

void JMatrix::sub(const jmatrix_t *m)
{
    int i,j;

    for (j=0;j<4;j++) {
        for (i=0;i<4;i++) {
            matrix.m[i][j]-=m->m[i][j];
        }
    }
}

void JMatrix::translate(float x, float y, float z)
{
	matrix.m[0][3]+=x;
	matrix.m[1][3]+=y;
	matrix.m[2][3]+=z;
}

void JMatrix::translate(const jpoint_t *p)
{
	matrix.m[0][3]+=p->x;
	matrix.m[1][3]+=p->y;
	matrix.m[2][3]+=p->z;
}

void JMatrix::scale(float s)
{
    int i,j;

    for (i=0;i<3;i++) {
        for (j=0;j<4;j++) {
            matrix.m[i][j]*=s;
        }
    }
}

void JMatrix::scale(float s, const jpoint_t *p)
{
	translate(-p->x,-p->y,-p->z);
	scale(s);
	translate(p);
}


/* Debug-Funktionen */
void JMatrix::dump()
{
    int i,j;
    
    cerr << "Matrix@" << this << " :\n";

    for (i=0;i<4;i++) {
        cerr << "  [ ";
        for (j=0;j<4;j++) {
            cerr << matrix.m[i][j] << " ";
        }
        cerr << "]\n";
    }
}

