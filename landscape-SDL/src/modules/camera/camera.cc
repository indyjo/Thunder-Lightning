#include "camera.h"
#include <modules/math/Matrix.h>
#include <modules/math/Vector.h>
#include <modules/environment/environment.h>

Camera::Camera(IGame *thegame)
{
    this->thegame=thegame;
    config=thegame->getConfig();
    this->focus=atof(config->query("Camera_focus","1.5"));
    this->aspect=atof(config->query("Camera_aspect","1.33333"));
}

Vector Camera::getLocation()
{
    return Vector(location);
}

Vector Camera::getFrontVector() {
    return Vector(front);
}

Vector Camera::getRightVector() {
    return Vector(right);
}

Vector Camera::getUpVector() {
    return Vector(up);
}

void Camera::getOrientation(Vector *up, Vector *right, Vector *front) {
    *up = Vector(this->up);
    *right = Vector(this->right);
    *front = Vector(this->front);
}

void Camera::alignWith(IPositionProvider *pos_provider)
{
    int i, j, r, c;
    /*
    The folowing variables have to be updated:
        * float location[3]
        * float front[3], right[3], up[3]
        * float planes[6][4]
        * JCamera cam

    front and right are provided by IPositionProvider but should be normalized
    (just to be sure...)
    up is the cross product of front * right
    */
    Vector loc, v_front, v_right, v_up;
    loc = pos_provider->getLocation();
    for(int i=0; i<3; i++) location[i]=loc[i];
    
    pos_provider->getOrientation(&v_up, &v_right, &v_front);
    //ls_warning("Camera: location %5.3f %5.3f %5.3f\n", location[0], location[1], location[2]);
    //ls_warning("Camera: front %5.3f %5.3f %5.3f\n", front[0], front[1], front[2]);
    //ls_warning("Camera: right %5.3f %5.3f %5.3f\n", right[0], right[1], right[2]);
    
    //ls_warning("Camera: up %5.3f %5.3f %5.3f\n", v_up[0], v_up[1], v_up[2]);
    for (i=0; i<3; i++) {
        front[i]=v_front[i];
        right[i]=v_right[i];
        up[i]=v_up[i];
    }
    
    /*
    The matrix M describes the rotation defined by the vectors right, up, front
    */
    float m[16] = {
            right[0], up[0], front[0], 0.0,
            right[1], up[1], front[1], 0.0,
            right[2], up[2], front[2], 0.0,
                 0.0,   0.0,      0.0, 1.0};
    Matrix M = Matrix::Array(m).transpose();
    // The normals of the viewing frustum, before transformation
    // Order: PLANE_MINUS_Z, PLANE_PLUS_Z,
    //        PLANE_MINUS_X, PLANE_PLUS_X,
    //        PLANE_MINUS_Y, PLANE_PLUS_Y
#define PLANE PLANE_PLUS_X
    Vector v_normals[6];
    float normals[6][3]={
        {  0.0,   0.0,    1.0}, {   0.0,    0.0,   -1.0},
        {focus,   0.0, aspect}, {-focus,    0.0, aspect},
        {  0.0, focus,    1.0}, {   0.0, -focus,    1.0}};

    for (i=0; i<6; i++) v_normals[i]=normals[i];
    /*ls_warning("Camera: normal %5.3f %5.3f %5.3f\n",
            normals[PLANE][0],
            normals[PLANE][1],
            normals[PLANE][2]);*/
    
    // Normalize the latter 4 vectors
    // (the first two don't need to be normalized)
    for (i=2; i<6; i++) v_normals[i].normalize();
    /*ls_warning("Camera: v_normal %5.3f %5.3f %5.3f\n",
            v_normals[PLANE][0],
            v_normals[PLANE][1],
            v_normals[PLANE][2]);*/
    
    // Apply the rotation matrix M on each of the vectors
    for (i=0; i<6; i++) v_normals[i] *= M;
    
    // Set the frustum plane values to the entries of the transformed normals.
    // Calculate the 4th value via the scalar product with the location vector

    Vector v_location(location);
    for (i=0; i<6; i++) {
        for (j=0; j<3; j++) planes[i][j]=v_normals[i][j];
        planes[i][3] = - (v_normals[i] * v_location);
    }
    
    // PLANE_PLUS_Z is the only plane that does not go through the camera
    // But since it is parallel to the screen we just have to add
    // CLIP_MAX_RANGE to its fourth entry
    planes[PLANE_PLUS_Z][3]+=thegame->getEnvironment()->getClipMax();
    
    // So much for the planes. Now on to the JCamera
    M.transpose();
    Vector v_camera_translation = - (M * v_location);
    float camera_matrix[4][4];
    for (c=0; c<4; c++) {
        for (r=0; r<4; r++) {
            camera_matrix[r][c] = M[c*4 + r];
        }
    }
    for (i=0; i<3; i++) camera_matrix[i][3]=v_camera_translation[i];
    for (i=0; i<4; i++) for(j=0; j<4; j++)
        cam.cam.matrix.m[i][j]  = camera_matrix[i][j];
    cam.cam.focus     = focus;
    // And that's it already
}


void Camera::getCamera(JCamera *camera)
{
    *camera = this->cam;
}

void Camera::getFrontBackPlane(float plane[4])
{
    int j;
    for(j=0;j<4; j++) plane[j] = this->planes[PLANE_MINUS_Z][j];
}

void Camera::getFrustumPlanes(float planes[6][4])
{
    int i,j;
    for(i=0;i<6;i++) {
        for(j=0;j<4;j++) planes[i][j]=this->planes[i][j];
    }
}

float Camera::getFocus() {
    return focus;
}

float Camera::getAspect() {
    return aspect;
}


