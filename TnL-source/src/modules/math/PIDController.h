#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

template<class T=float>
class PIDController
{
    float Ki, Kd, Kp;
    T last_error, error_integral;
    bool init;
    
public:
    PIDController(float Kp, float Kd=0, float Ki=0);
    T control(const T & error, float dt);
    void reset();
};



#endif
