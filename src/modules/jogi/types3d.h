#ifndef _JOGI_TYPES3D_H
#define _JOGI_TYPES3D_H

struct jmatrix_t {
    float m[4][4];  /* m[row][column] */
};

struct jpoint_t {
	float x,y,z;
};

struct jpoint4_t {
    float x,y,z,w;
};

struct jcolor3_t {
    float r,g,b;
};

struct jvertex_col {
    jpoint_t   p;
    jcolor3_t col;
};

struct jvertex_txt {
    jpoint_t p;
    jpoint_t txt;
};

struct jvertex_coltxt {
    jpoint_t   p;
    jcolor3_t  col;
    jpoint_t   txt;
};

struct jcamera_t {
    jmatrix_t matrix;
    float focus, aspect;
};

#endif
