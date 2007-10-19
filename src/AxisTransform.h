#ifndef AXISTRANSFORM_H
#define AXISTRANSFORM_H

#include <vector>
#include <cmath>
#include <object.h>

/// An operation that takes an arbitrary number of inputs (containing float
/// values) and outputs a single float. AxisTransform is used by EventRemapper
/// to transform input from mouse and joystick axes.
struct AxisTransform : public Object {
    virtual float operator() (std::vector<float> & inputs) = 0;
};

class LinearAxisTransform : public AxisTransform {
    float a,b;
public:
    LinearAxisTransform(float a=1.0f, float b=0.0f) : a(a), b(b) { }
    virtual float operator() (std::vector<float> & inputs) { return a*inputs[0]+b; }
};

struct SumAxesTransform : public AxisTransform {
    virtual float operator() (std::vector<float> & inputs) {
        float out = 0.0f;
        for(int i=0; i<inputs.size(); i++) out += inputs[i];
        return out;
    }
};

class ClampAxisTransform : public AxisTransform {
    float a, b;
public:
    ClampAxisTransform(float a=-1.0f, float b=1.0f) : a(a), b(b) { }
    virtual float operator() (std::vector<float> & inputs) {
        return (inputs[0]<a)?a:((inputs[0]>b)?b:inputs[0]);
    }
};

class SelectAxisByActivityTransform : public AxisTransform {
	std::vector<float> old_values;
	float threshold;
	int active;
	bool init;
public:
	SelectAxisByActivityTransform(float threshold = 0.0f, float init_value=0.0f);
	virtual float operator() (std::vector<float> & inputs);
};

class SensitivityAxisTransform : public AxisTransform {
    float s;
public:
    SensitivityAxisTransform(float s) : s(s) { }
    virtual float operator() (std::vector<float> & inputs)
    { return inputs[0] * pow(std::abs(inputs[0]), s); }
};

class JoystickAxisTransform : public AxisTransform {
    float threshold, deadzone;
    bool nonnegative, inverted;
public:
    JoystickAxisTransform(bool nonnegative, bool inverted=false, float threshold=0, float deadzone=0)
    : nonnegative(nonnegative)
    , inverted(inverted)
    , threshold(threshold)
    , deadzone(deadzone)
    { }
    
    virtual float operator() (std::vector<float> & inputs);
};

#endif

