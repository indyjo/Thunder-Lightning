#include <cassert>
#include <algorithm>
#include "ContinuousStateVector.h"

ContinuousStateVector::Iter::Iter(ContinuousStateVector * vec, const Floats::iterator & floats_iter)
    : vec(vec)
    , floats_iter(floats_iter)
{ }

void ContinuousStateVector::Iter::writeFloats(const float* values, int count) {
    // Overwrite if not at end, extend otherwise
    
    if (floats_iter != vec->floats.end()) {
        int tocopy = vec->floats.end() - floats_iter;
        floats_iter = std::copy(values, values+tocopy, floats_iter);
        values += tocopy;
        count -= tocopy;
    }
    
    assert(floats_iter == vec->floats.end());
    
    // extend vector by remaining elements
    if (count > 0) {
        vec->floats.insert(floats_iter, values, values+count);
        floats_iter += count;
    }
}

void ContinuousStateVector::Iter::readFloats(float * values, int count) {
    std::copy(floats_iter, floats_iter+count, values);
}

void ContinuousStateVector::Iter::truncate() {
    vec->floats.erase(floats_iter, vec->floats.end());
    assert(floats_iter == vec->floats.end());
}

ContinuousStateVector::Iter ContinuousStateVector::begin() {
    return Iter(this, floats.begin());
}

ContinuousStateVector::Iter ContinuousStateVector::end() {
    return Iter(this, floats.end());
}
