#include "camera.h"
#include <modules/math/SpecialMatrices.h>
#include <modules/environment/environment.h>
#include <interfaces/IConfig.h>

Camera::Camera(IGame *thegame)
:	dirty(true)
{
    this->thegame=thegame;
    config=thegame->getConfig();
    this->focus=atof(config->query("Camera_focus","1.5"));
    this->aspect=atof(config->query("Camera_aspect","1.33333"));
    orient = IdentityMatrix<3,float>();
    location = Vector(0,0,0);
}

Vector Camera::getLocation()
{
    return location;
}

Vector Camera::getFrontVector() {
    return orient*Vector(0,0,1);
}

Vector Camera::getRightVector() {
    return orient*Vector(1,0,0);
}

Vector Camera::getUpVector() {
    return orient*Vector(0,1,0);
}

void Camera::getOrientation(Vector *up, Vector *right, Vector *front) {
	*up = Vector(orient(0,1),orient(1,1),orient(2,1));
	*right = Vector(orient(0,0),orient(1,0),orient(2,0));
	*front = Vector(orient(0,2),orient(1,2),orient(2,2));
}

void Camera::setLocation(const Vector & p) {
	location = p;
	dirty = true;
}

void Camera::setOrientation(const Vector & up,
  							const Vector & right,
							const Vector & front)
{
	dirty = true;
	orient = Matrix3(
		right[0], up[0], front[0],
		right[1], up[1], front[1],
		right[2], up[2], front[2]);
}

void Camera::alignWith(IPositionProvider *pos_provider)
{
	Vector p,up,right,front;
	p=pos_provider->getLocation();
	pos_provider->getOrientation(&up,&right,&front);
	setLocation(p);
	setOrientation(up,right,front);
}

void Camera::getCamera(JCamera *camera)
{
	if (dirty) update();
	camera->cam = jcam.cam;
}

void Camera::getFrontBackPlane(float plane[4])
{
    if (dirty) update();
    for(int j=0;j<4; j++)
    	plane[j] = this->planes[PLANE_MINUS_Z][j];
}

void Camera::getFrustumPlanes(float planes[6][4])
{
    if (dirty) update();
    for(int i=0;i<6;i++) {
        for(int j=0;j<4;j++)
        	planes[i][j]=this->planes[i][j];
    }
}

float Camera::getFocus() {
    return focus;
}

float Camera::getAspect() {
    return aspect;
}

const Matrix3 & Camera::getOrient() {
	return orient;
}

const Matrix3 & Camera::getOrientInv() {
	if (dirty) update();
	return orient_inv;
}

void Camera::update() {
	dirty = false;
	orient_inv = orient;
	orient_inv.transpose();
	
    // The normals of the viewing frustum, before transformation
    // Order: PLANE_MINUS_Z, PLANE_PLUS_Z,
    //        PLANE_MINUS_X, PLANE_PLUS_X,
    //        PLANE_MINUS_Y, PLANE_PLUS_Y
    float normals_init[6][3]={
        {  0.0,   0.0,    1.0}, {   0.0,    0.0,   -1.0},
        {focus,   0.0, aspect}, {-focus,    0.0, aspect},
        {  0.0, focus,    1.0}, {   0.0, -focus,    1.0}};
    Vector normals[6];
    for(int i=0; i<6;++i)
    	normals[i]=orient*Vector(normals_init[i]);
    for(int i=2; i<6;++i)
    	normals[i].normalize();
    
    // X and Y planes touch the camera location
    for(int i=2; i<6;++i)
    	planes[i] = Plane(location,normals[i]);
    // Z planes depend on clipping depth
    float clip_min, clip_max;
    if (thegame->getEnvironment()) {
    	clip_min = thegame->getEnvironment()->getClipMin();
    	clip_max = thegame->getEnvironment()->getClipMax();
    } else {
    	clip_min = 0.1;
    	clip_max = 10;
    }
    planes[PLANE_MINUS_Z] = Plane(
		location + orient * Vector(0,0,clip_min),
		normals[PLANE_MINUS_Z]);
    planes[PLANE_PLUS_Z] = Plane(
		location + orient * Vector(0,0,clip_max),
		normals[PLANE_PLUS_Z]);
    		
    // Now calculate the camera Matrix
    Matrix M = Matrix::Hom(orient_inv, -orient_inv*location);
    
    for(int i=0;i<4;++i) for(int j=0;j<4;++j)
    	jcam.cam.matrix.m[i][j] = M(i,j);
    jcam.cam.focus = focus;
    
    // That's it!
}
