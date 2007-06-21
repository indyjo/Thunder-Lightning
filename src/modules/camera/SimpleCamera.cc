#include <modules/math/SpecialMatrices.h>
#include "SimpleCamera.h"

SimpleCamera::SimpleCamera()
{
    focus=1.5;
    aspect=1.333333f;
    orient = IdentityMatrix<3,float>();
    location = Vector(0,0,0);
    near_dist = 1.0f;
    far_dist = 1000.0f;
    dirty = true;
}

SimpleCamera::SimpleCamera(Ptr<ICamera> other)
{
    this->focus=other->getFocus();
    this->aspect=other->getAspect();
    orient = other->getOrient();
    location = other->getLocation();
    near_dist = other->getNearDistance();
    far_dist = other->getFarDistance();
    dirty = true;
}


void SimpleCamera::setFocus(float val) {
    this->focus = val;
    dirty = true;
}

void SimpleCamera::setAspect(float val) {
    this->aspect = val;
    dirty = true;
}

void SimpleCamera::setNearDistance(float val) {
    this->near_dist = val;
    dirty = true;
}

void SimpleCamera::setFarDistance(float val) {
    this->far_dist = val;
    dirty = true;
}

Vector SimpleCamera::getLocation()
{
    return location;
}

Vector SimpleCamera::getFrontVector() {
    return orient*Vector(0,0,1);
}

Vector SimpleCamera::getRightVector() {
    return orient*Vector(1,0,0);
}

Vector SimpleCamera::getUpVector() {
    return orient*Vector(0,1,0);
}

void SimpleCamera::getOrientation(Vector *up, Vector *right, Vector *front) {
	*up = Vector(orient(0,1),orient(1,1),orient(2,1));
	*right = Vector(orient(0,0),orient(1,0),orient(2,0));
	*front = Vector(orient(0,2),orient(1,2),orient(2,2));
}

void SimpleCamera::setLocation(const Vector & p) {
	location = p;
	dirty = true;
}

void SimpleCamera::setOrientation(const Vector & up,
  							const Vector & right,
							const Vector & front)
{
	dirty = true;
	orient = Matrix3(
		right[0], up[0], front[0],
		right[1], up[1], front[1],
		right[2], up[2], front[2]);
}

void SimpleCamera::alignWith(IPositionProvider *pos_provider)
{
	Vector p,up,right,front;
	p=pos_provider->getLocation();
	pos_provider->getOrientation(&up,&right,&front);
	setLocation(p);
	setOrientation(up,right,front);
}

void SimpleCamera::getCamera(JCamera *camera)
{
	if (dirty) update();
	camera->cam = jcam.cam;
}

void SimpleCamera::getFrontBackPlane(float plane[4])
{
    if (dirty) update();
    for(int j=0;j<4; j++)
    	plane[j] = this->planes[PLANE_MINUS_Z][j];
}

void SimpleCamera::getFrustumPlanes(float planes[6][4])
{
    if (dirty) update();
    for(int i=0;i<6;i++) {
        for(int j=0;j<4;j++)
        	planes[i][j]=this->planes[i][j];
    }
}

float SimpleCamera::getFocus() {
    return focus;
}

float SimpleCamera::getAspect() {
    return aspect;
}

const Matrix3 & SimpleCamera::getOrient() {
	return orient;
}

const Matrix3 & SimpleCamera::getOrientInv() {
	if (dirty) update();
	return orient_inv;
}

float SimpleCamera::getNearDistance() {
    return near_dist;
}

float SimpleCamera::getFarDistance() {
    return far_dist;
}

void SimpleCamera::update() {
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
    planes[PLANE_MINUS_Z] = Plane(
		location + orient * Vector(0,0,near_dist),
		normals[PLANE_MINUS_Z]);
    planes[PLANE_PLUS_Z] = Plane(
		location + orient * Vector(0,0,far_dist),
		normals[PLANE_PLUS_Z]);
    		
    // Now calculate the camera Matrix
    Matrix M = Matrix::Hom(orient_inv, -orient_inv*location);
    
    for(int i=0;i<4;++i) for(int j=0;j<4;++j)
    	jcam.cam.matrix.m[i][j] = M(i,j);
    jcam.cam.focus = focus;
    
    // That's it!
}

