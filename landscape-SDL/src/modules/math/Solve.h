#ifndef SOLVE_H
#define SOLVE_H

#include <complex>

namespace Solve {
    // solve at + b = 0
    bool order1(float a, float b, float * t);
        
    // solve at^2 + bt + c = 0
    // t1 <= t2
    bool order2(float a, float b, float c, float * t1, float * t2);
    // solve using complex numbers.
    // Returns true if solutions are complex
    bool order2complex(float a, float b, float c, 
            std::complex<float> * t1, std::complex<float> * t2);
    
    // solve "depressed" cubic formula
    // y^3 + Ay = B
    // returns number of real solutions (n=1 or 3)
    // side effects: solutions get written to the float array pointed to by y
    //               values y[1] and y[2] are only overwritten when n = 3
    //               value y[0] will always be overwritten
    int order3depressed(float A, float B, float *y);
    
    // solve generic cubic formula
    // at^3 + bt^2 + ct + d = 0
    // returns number of real solutions (n=0..3)
    // side effects: solutions get written to the float array pointed to by y
    //               values y[1] and y[2] are only overwritten when n = 3
    //               value y[0] will always be overwritten
    int order3(float a, float b, float c, float d, float *y);
}

#endif
