#include <cmath>
#include "Solve.h"


namespace Solve {

using namespace std;

// solve ax + b = 0
bool order1(float a, float b, float * t) {
    if (b!=0) {
        if (a != 0) {
            *t = -b/a;
            return true;
        } else return false;
    } else {
        return true;
        *t=0;
    }
}


// solve ax^2 + bx + c = 0
bool order2(float a, float b, float c, float * t1, float * t2) {
    if (a==0) { // not a real order 2 polynom
        if (b == 0) { // not even an order 1 polynom
            if (c == 0) {
                *t1 = *t2 = 0;
                return true;
            } else return false;
        }
        *t1 = *t2 = - c / b;
        return true;
    }
    float D = b*b - 4.0*a*c;
    if ( D >= 0 ) {
        D = sqrt(D);
        a *= 2.0;
        *t1 = (-b - D) / a;
        *t2 = (-b + D) / a;
        return true;
    }
    return false;
}

bool order2complex(float a, float b, float c,
        complex<float> * t1, complex<float> * t2) {
    if (a==0) { // not a real order 2 polynom
        if (b == 0) { // not even an order 1 polynom
            *t1 = *t2 = 0;
            return false;
        }
        *t1 = *t2 = - c / b;
        return false;
    }
    complex<float> D = b*b - 4.0*a*c;
    bool is_complex = D.real() < 0;
    D = sqrt(D);
    a *= 2.0;
    *t1 = (-b - D) / a;
    *t2 = (-b + D) / a;
    return is_complex;
}

// solve y^3 + Ay = B
// see http://www.sosmath.com/algebra/factor/fac11/fac11.html for reference
int order3depressed(float A, float B, float *y) {
    complex<float> u1, u2;
    bool irreducible = order2complex(1.0, B, - A*A*A/27.0, &u1, &u2);
    complex<float> t = pow(u2, 1.0f/3);
    complex<float> s = A / (3.0f*t);

    y[0] = (s - t).real();
    if (!irreducible) return 1;

    // Casus Irreducibilis ==> 3 real solutions
    // the other two solutions can be found using polynomial division and
    // solving the resulting order-2 equation
    order2(1.0, y[0], A + y[0]*y[0], &y[1], &y[2]);

    return 3;
}

// solve at^3 + bt^2 + ct + d = 0 by converting into "depressed" formula
int order3(float a, float b, float c, float d, float *y) {
    if (a==0.0) { // Not a cubic equation
        if (order2(b,c,d,&y[0], &y[1])) return 2;
        else return 0;
    }

    float A = (c - b*b/(3.0*a)) / a;
    float B = (-d - 2.0*b*b*b/(27.0*a*a) + b*c/(3.0*a)) / a;
    return order3depressed(A, B, y);
}

} // namespace Solve
