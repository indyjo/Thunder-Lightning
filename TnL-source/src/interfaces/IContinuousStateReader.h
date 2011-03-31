#ifndef ICONTINUOUSSTATEREADER_H
#define ICONTINUOUSSTATEREADER_H

#include <modules/math/Transform.h>
#include <modules/math/Vector.h>

struct IContinuousStateReader {
    virtual void readFloats(float * values, int count)=0;
    
    inline void readFloat(float & value) {
        readFloats(&value, 1);
    }
    
    inline float readFloat() {
        float f;
        readFloat(f);
        return f;
    }
    
    template<int N>
    void readVector(XVector<N,float> &vec) {
        readFloats(vec.raw(), N);
    }
    
    inline void readQuaternion(Quaternion & quat) {
        readFloat(quat.real());
        readVector(quat.imag());
    }
    
    inline void readTransform(Transform & xform) {
        readQuaternion(xform.quat());
        readVector(xform.vec());
    }
};

#endif
