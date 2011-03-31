#include "AxisTransform.h"
#include <debug.h>

SelectAxisByActivityTransform::SelectAxisByActivityTransform(
	float threshold, float init_value)
:	threshold(threshold), init(true), active(0)
{ }
	
float SelectAxisByActivityTransform::operator() (std::vector<float> & inputs) {
    if (inputs.size() != old_values.size()) {
        init = true;
    }
    
    if (init) {
	    old_values = inputs;
	    init = false;
	    active = 0;
    } else {
		for (int i=0; i<inputs.size(); ++i) {
			if (std::abs(inputs[i]-old_values[i]) > threshold) {
			    active = i;
			    break;
			}
		}
		old_values=inputs;
	}
    
    return inputs[active];
}

float JoystickAxisTransform::operator() (std::vector<float> & inputs) {
    float in = inputs[0];
    if (inverted) in = -in;
    
    float result = 0;
    if (nonnegative) {
        if (in <= -1+threshold) {
            result = 0;
        } else if (in < 1-threshold) {
            result = (in - (-1+threshold)) / (2-2*threshold);
        } else {
            result = 1;
        }
    } else {
        if (in <= -1+threshold) {
            result = -1;
        }else if (in < -deadzone) {
            result = -1 + (in - (-1+threshold)) / (-deadzone - (-1+threshold));
        } else if (in <= deadzone) {
            result = 0;
        } else if (in < 1-threshold) {
            result = (in - threshold) / (1-threshold - deadzone);
        } else {
            result = 1;
        }
    }
    
    return result;
}

