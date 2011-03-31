#ifndef _JOGI_MATERIAL_H
#define _JOGI_MATERIAL_H

#include <modules/math/Vector.h>
#include <object.h>

struct JMaterial : public Object {
	virtual void activate() = 0;
	
	virtual void setDiffuse(const Vector &) = 0;
	virtual void setSpecular(const Vector &) = 0;
	virtual void setAmbient(const Vector &) = 0;
	virtual void setAmbientAndDiffuse(const Vector &) = 0;
	virtual void setEmission(const Vector &) = 0;
	
	virtual void setShininess(float) = 0;
};

#endif
