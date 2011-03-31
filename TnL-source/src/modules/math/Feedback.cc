#include "Feedback.h"

template <class T, class M>
T FeedbackController<T,M>::control(const T & x, const T & dx) {
    return Kd * (x_target - x) + Kv * dx;
}

// Explicit instantiations
template class FeedbackController<float>;
