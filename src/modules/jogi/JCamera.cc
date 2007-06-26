#include <cmath>
#include "jogi.h"

JCamera::JCamera()
{
    init();
}


JCamera::JCamera(jcamera_t *cam)
{
    init(cam);
}


JCamera::JCamera(jpoint_t *pos, jpoint_t *rot, float focus)
{
    init(pos,rot,focus);
}


void JCamera::init()
{
    JMatrix m;
    m.init();
    cam.matrix=m.matrix;
    cam.focus=1;
    cam.aspect=1;
}


void JCamera::init(jcamera_t *cam)
{
    this->cam=*cam;
}


void JCamera::initPoV(jpoint_t *pos, jpoint_t *look_at, float focus)
{
    // TODO!
}


void JCamera::initObjectMatrix(const jmatrix_t *matrix)
{
    JMatrix m1,m2,m3;
    m1.init(matrix);
    m1.translate(-matrix->m[0][3],-matrix->m[1][3],-matrix->m[2][3]);
    m2.init();
    for (int i=0;i<3;i++) {
        for (int j=0;j<3;j++) {
            m2.matrix.m[j][i]=matrix->m[i][j];
        }
    }
    m3.initMultiplyBA(&m1.matrix,&m2.matrix);
    cam.matrix=m3.matrix;
}
    

void JCamera::init(jpoint_t *pos, jpoint_t *rot, float focus)
{
    JMatrix m1,m2,m3;

    m1.initTranslate(-pos->x,-pos->y,-pos->z);
    m2.initRotateZ(-rot->z);
    m3.initMultiplyBA(&m1.matrix, &m2.matrix);

    m2.initRotateY(-rot->y);
    m1.initMultiplyBA(&m3.matrix, &m2.matrix);

    m2.initRotateX(-rot->x);
    m3.initMultiplyBA(&m1.matrix, &m2.matrix);

    cam.matrix=m3.matrix;
    cam.focus=focus;
}


void JCamera::getEyeCoords(jpoint_t *world, jpoint_t *eye)
{
    JPoint p(world);

    p.applyMatrix(&cam.matrix);
    *eye=p.point;
}


