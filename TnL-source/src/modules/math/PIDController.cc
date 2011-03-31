#include "PIDController.h"
#include <modules/math/Vector.h>

template<class T>
PIDController<T>::PIDController(float Kp, float Kd, float Ki)
: 	Ki(Ki), Kd(Kd), Kp(Kp),
	last_error(0), error_integral(0),
	init(true)
{ }

template<class T>
T PIDController<T>::control(const T & error, float dt) {
	if (init) {
		last_error = error;
		init = false;
		return Kp * error;
	} else {
		error_integral += 0.5f*(error+last_error)*dt;
		T error_derivative = (error-last_error)/dt;
		last_error = error;
		return Kp * error +
			   Kd * error_derivative +
			   Ki * error_integral;
	}
}

template<class T>
void PIDController<T>::reset() {
	init = true;
	error_integral=0;
	last_error=0;
}

// Explicit instantiations
template class PIDController<float>;
