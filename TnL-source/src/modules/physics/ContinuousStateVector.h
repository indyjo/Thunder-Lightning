#ifndef CONTINUOUSSTATEVECTOR_H
#define CONTINUOUSSTATEVECTOR_H

#include <vector>
#include <interfaces/IContinuousStateReader.h>
#include <interfaces/IContinuousStateWriter.h>

class ContinuousStateVector {
    friend class Iter;
    
    typedef std::vector<float> Floats;
    
    Floats floats;
    
public:
    ContinuousStateVector();
    
    /// Initializes an empty vector with the given preallocated reserve.
    ContinuousStateVector(int size_hint);
    
    
    class Iter : public IContinuousStateReader, public IContinuousStateWriter {
        friend class ContinuousStateVector;
        
        ContinuousStateVector * vec;
        
        Floats::iterator floats_iter;
        
        Iter(ContinuousStateVector * vec, const Floats::iterator & floats_iter);

    public:
        virtual void writeFloats(const float* values, int count);
        virtual void readFloats(float * values, int count);
        void truncate();
    };

    Iter begin();
    Iter end();
};

#endif
