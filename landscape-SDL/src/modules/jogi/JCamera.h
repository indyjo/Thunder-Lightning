#ifndef _JOGI_CAMERA_H
#define _JOGI_CAMERA_H

class JCamera
{
public:
    jcamera_t cam;

    /* Constructors and Initialisation methods ----------------------*/

    /*
     Initializes to default values
     */
    JCamera();

    /*
     Initializes with values of another camera object
     */
    JCamera(jcamera_t *cam);

    /*
     Initializes with a position and a rotation
     */
    JCamera(jpoint_t *pos, jpoint_t *rot, float focus);
    
    /*
     Re-initializes to default values
     */
    void init();

    /*
     Re-Initializes with values of another camera object
     */
    void init(jcamera_t *cam);

    /*
     Re-Initializes with a position and a rotation
     */
    void init(jpoint_t *pos, jpoint_t *rot, float focus);
    
    /*
     Re-Initializes with PoV-Ray position/look_at scheme
     */
    void initPoV(jpoint_t *pos, jpoint_t *look_at, float focus);

    /*
     Initializes the camera with a matrix like one of an object
     */
    void initObjectMatrix(const jmatrix_t *matrix);

    /* Projection methods -------------------------------------------*/

    void getEyeCoords(jpoint_t *world, jpoint_t *eye);

};
    

#endif
