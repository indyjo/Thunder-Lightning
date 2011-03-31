#include <cmath>
#include <iostream>
#include "jogi.h"


using namespace std;


/* Konstruktoren ----------------------------------------------------------*/

JPoint::JPoint()
{
	point.x=0.0f;
	point.y=0.0f;
	point.z=0.0f;
}

JPoint::JPoint(const jpoint_t *p)
{
	point.x=p->x;
	point.y=p->y;
	point.z=p->z;
}

JPoint::JPoint(float x, float y, float z)
{
	point.x=x;
	point.y=y;
	point.z=z;
}

/* Initialisierungsfunktionen ---------------------------------------------*/

void JPoint::clear()
{
	point.x=0;
	point.y=0;
	point.z=0;
}

void JPoint::set(const jpoint_t *p)
{
	point.x=p->x;
	point.y=p->y;
	point.z=p->z;
}

void JPoint::set(float x, float y, float z)
{
	point.x=x;
	point.y=y;
	point.z=z;
}

/* Operatoren -------------------------------------------------------------*/
void JPoint::add(const jpoint_t *add)
{
	point.x+=add->x;
	point.y+=add->y;
	point.z+=add->z;
}

void JPoint::add(float addx, float addy, float addz)
{
	point.x+=addx;
	point.y+=addy;
	point.z+=addz;
}

void JPoint::sub(const jpoint_t *sub)
{
	point.x-=sub->x;
	point.y-=sub->y;
	point.z-=sub->z;
}

void JPoint::sub(float subx, float suby, float subz)
{
	point.x-=subx;
	point.y-=suby;
	point.z-=subz;
}

void JPoint::scale(float s)
{
	point.x*=s;
	point.y*=s;
	point.z*=s;
}

void JPoint::scale(float s, const jpoint_t *p)
{
	sub(p);
	scale(s);
	add(p);
}

void JPoint::rotateX(float a)
{
	float y2;
	float ca=cos(a);
	float sa=sin(a);
	
	y2=point.y*ca-point.z*sa;
	point.z=point.y*sa+point.z*ca;
	point.y=y2;
}

void JPoint::rotateY(float a)
{
	float x2;
	float ca=cos(a);
	float sa=sin(a);
	
	x2=point.x*ca+point.z*sa;
	point.z=-point.x*sa+point.z*ca;
	point.x=x2;
}

void JPoint::rotateZ(float a)
{
	float x2;
	float ca=cos(a);
	float sa=sin(a);
	
	x2=point.x*ca-point.y*sa;
	point.y=point.x*sa+point.y*ca;
	point.x=x2;
}

void JPoint::rotate(float a, float b, float c)
{
	float x2,y2,z2;
	float ca=cos(a);
	float sa=sin(a);
	float cb=cos(b);
	float sb=sin(b);
	float cc=cos(c);
	float sc=sin(c);
	float sasb=sa*sb;
	float casb=ca*sb;
	
	x2=point.x*(cb*cc) + point.y*(sasb*cc-ca*sc) + point.z*(casb*cc+sa*sc);
	y2=point.x*(cb*sc) + point.y*(sasb*sc+ca+cc) + point.z*(casb*sc-sa*cc);
	z2=point.x*(-sb)   + point.y*(sa*cb)         + point.z*(ca*cb);
	point.x=x2;
	point.y=y2;
	point.z=z2;
}

void JPoint::applyMatrix(const jmatrix_t *m)
{
	float x2,y2,z2;
	x2=point.x*m->m[0][0] + point.y*m->m[0][1] + point.z*m->m[0][2] +m->m[0][3];
	y2=point.x*m->m[1][0] + point.y*m->m[1][1] + point.z*m->m[1][2] +m->m[1][3];
	z2=point.x*m->m[2][0] + point.y*m->m[2][1] + point.z*m->m[2][2] +m->m[2][3];
	point.x=x2;
	point.y=y2;
	point.z=z2;
}

float JPoint::getLength()
{
    return sqrt(point.x*point.x+
                point.y*point.y+
                point.z*point.z);
}

void JPoint::normalize()
{
    float length=sqrt(point.x*point.x+
                      point.y*point.y+
                      point.z*point.z);

    point.x/=length;
    point.y/=length;
    point.z/=length;
}

void JPoint::vcross(const jpoint_t *p1, const jpoint_t *p2)
{
    point.x=p1->y*p2->z - p2->y*p1->z;
    point.y=p2->x*p1->z - p1->x*p2->z;
    point.z=p1->x*p2->y - p2->x*p1->y;
}

void JPoint::dump(ostream & out)
{
	out << "JPoint@" << this << ": (";
	out << point.x << " | " << point.y << " | " << point.z << ")\n";
}
