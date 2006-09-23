#ifndef FEEDBACK_H
#define FEEDBACK_H

template<class T, class M=T>
class FeedbackController
{
    T x_target;
    M Kd, Kv;
    
public:
    inline FeedbackController(const T & x_target, const M & Kd, const M & Kv)
    : x_target(x_target), Kd(Kd), Kv(Kv)
    { }
    
    T control(const T & x, const T & dx);
};

#endif
