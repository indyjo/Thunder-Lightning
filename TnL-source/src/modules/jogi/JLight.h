#ifndef _JOGI_LIGHT_H
#define _JOGI_LIGHT_H

#include <modules/math/Vector.h>
#include <object.h>

struct JLight : public Object {
	virtual void setEnabled(bool) = 0;
	virtual bool getEnabled() = 0;
	
	virtual void setColor(const Vector &) = 0;
};

struct JAttenuatedLight : public JLight {
	virtual void setAttenuation(float squared, float linear, float constant) = 0;
};

struct JDirectionalLight : public JLight {
	virtual void setDirection(const Vector &) = 0;
};

struct JPointLight : public JAttenuatedLight {
	virtual void setPosition(const Vector &) = 0;
};

#endif
