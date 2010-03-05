#include <modules/math/SpecialMatrices.h>
#include "SimpleCamera.h"

SimpleCameraBase::SimpleCameraBase() {
    focus=1.5;
    aspect=1.333333f;
    near_dist = 1.0f;
    far_dist = 1000.0f;
}

SimpleCameraBase::~SimpleCameraBase()
{ }

void SimpleCameraBase::setFocus(float val) {
    this->focus = val;
}

void SimpleCameraBase::setAspect(float val) {
    this->aspect = val;
}

void SimpleCameraBase::setNearDistance(float val) {
    this->near_dist = val;
}

void SimpleCameraBase::setFarDistance(float val) {
    this->far_dist = val;
}

Vector SimpleCameraBase::getFrontVector() {
    Vector up, right, front;
    getOrientation(&up, &right, &front);
    return front;
}

Vector SimpleCameraBase::getRightVector() {
    Vector up, right, front;
    getOrientation(&up, &right, &front);
    return right;
}

Vector SimpleCameraBase::getUpVector() {
    Vector up, right, front;
    getOrientation(&up, &right, &front);
    return up;
}

float SimpleCameraBase::getFocus() {
    return focus;
}

float SimpleCameraBase::getAspect() {
    return aspect;
}

float SimpleCameraBase::getNearDistance() {
    return near_dist;
}

float SimpleCameraBase::getFarDistance() {
    return far_dist;
}

Matrix3 SimpleCameraBase::getOrient() {
    Vector up, right, front;
    getOrientation(&up, &right, &front);
    return MatrixFromColumns(right, up, front);
}

Matrix3 SimpleCameraBase::getOrientInv() {
    Matrix3 orient = getOrient();
    orient.transpose();
    return orient;
}

void SimpleCameraBase::getCamera(JCamera *out) {
    Matrix3 orient_inv = getOrientInv();
    
    Matrix M = Matrix::Hom(orient_inv, -orient_inv*getLocation());
    
    for(int i=0;i<4;++i) for(int j=0;j<4;++j)
    	out->cam.matrix.m[i][j] = M(i,j);
    out->cam.focus = getFocus();
    out->cam.aspect = getAspect();
}

void SimpleCameraBase::getFrontBackPlane(float plane[4]) {
    Vector front = getFrontVector();
    Plane p(getLocation() + getFarDistance()*front, front);
    for(int i=0; i<4; ++i) plane[i] = p[i];
}

void SimpleCameraBase::getFrustumPlanes(float out_planes[6][4]) {
    Matrix3 orient = getOrient();
    Vector location = getLocation();
    float focus = getFocus();
    float aspect = getAspect();
    float near_dist = getNearDistance();
    float far_dist = getFarDistance();
    
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
    Plane planes[6];
    for(int i=2; i<6;++i)
    	planes[i] = Plane(location,normals[i]);
    
    // Z planes depend on clipping depth
    planes[PLANE_MINUS_Z] = Plane(
		location + orient * Vector(0,0,near_dist),
		normals[PLANE_MINUS_Z]);
    planes[PLANE_PLUS_Z] = Plane(
		location + orient * Vector(0,0,far_dist),
		normals[PLANE_PLUS_Z]);
    
    for(int i=0;i<6;i++) {
        for(int j=0;j<4;j++)
        	out_planes[i][j]=planes[i][j];
    }
}


SimpleCamera::SimpleCamera()
{
    orient = IdentityMatrix<3,float>();
    location = velocity = Vector(0);
}

SimpleCamera::SimpleCamera(Ptr<ICamera> other)
{
    *this = *other;
}

SimpleCamera & SimpleCamera::operator= (ICamera & other) {
    focus=other.getFocus();
    aspect=other.getAspect();
    orient = other.getOrient();
    location = other.getLocation();
    near_dist = other.getNearDistance();
    far_dist = other.getFarDistance();
    return *this;
}

Vector SimpleCamera::getLocation()
{
    return location;
}

void SimpleCamera::getOrientation(Vector *up, Vector *right, Vector *front) {
	*up = Vector(orient(0,1),orient(1,1),orient(2,1));
	*right = Vector(orient(0,0),orient(1,0),orient(2,0));
	*front = Vector(orient(0,2),orient(1,2),orient(2,2));
}

Vector SimpleCamera::getMovementVector() {
    return velocity;
}

void SimpleCamera::setLocation(const Vector & p) {
	location = p;
}

void SimpleCamera::setOrientation(const Vector & up,
  							const Vector & right,
							const Vector & front)
{
	orient = Matrix3(
		right[0], up[0], front[0],
		right[1], up[1], front[1],
		right[2], up[2], front[2]);
}

void SimpleCamera::setMovementVector(const Vector & v) {
    velocity = v;
}

void SimpleCamera::alignWith(IMovementProvider *provider)
{
	Vector p,v,up,right,front;
	p=provider->getLocation();
	v=provider->getMovementVector();
	provider->getOrientation(&up,&right,&front);
	setLocation(p);
	setMovementVector(v);
	setOrientation(up,right,front);
}

void SimpleCamera::pointTo(Vector target) {
    Vector sky(0,1,0);
    Vector front = (target - location).normalize();
    Vector right = (sky % front).normalize();
    Vector up = front % right;
    setOrientation(up,right,front);
}

