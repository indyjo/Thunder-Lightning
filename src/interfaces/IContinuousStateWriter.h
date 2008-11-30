#ifndef ICONTINUOUSSTATEWRITER_H
#define ICONTINUOUSSTATEWRITER_H

#include <modules/math/Transform.h>
#include <modules/math/Vector.h>

struct IContinuousStateWriter {
    virtual void writeFloats(const float* values, int count)=0;
    
    inline void writeFloat(float f) {
        writeFloats(&f, 1);
    }
    
    template<int N>
    void writeVector(const XVector<N, float> & vec) {
        writeFloats(vec.raw(), N);
    }

    inline void writeQuaternion(const Quaternion & quat) {
        writeFloat(quat.real());
        writeVector(quat.imag());
    }
    
    void writeTransform(const Transform & xform) {
        writeQuaternion(xform.quat());
        writeVector(xform.vec());
    }
};

#endif
