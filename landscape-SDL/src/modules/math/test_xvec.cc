#include <iostream>
#include "Vector.h"
#include "Matrix.h"
#include "Interval.h"

using namespace std;

int main() {
    Vector p(1,2,3);
    Vector q(5,6,7);    
    float r = p * q;
    cout << p << " * " << q << " = " << r << endl;
    cout << " r * Vector(3,2,1) = " << r * Vector(3,2,1) << endl;
    Vector s = r * Vector(3,2,1);
    cout << r << " * " << Vector(3,2,1) << " = " << s << endl;
    Vector t = s / s.lengthSquare();
    cout << t << endl;
    
    cout << endl;
    
    typedef XVector<3, Interval> IVector;
    
    IVector x(Interval(0.9, 1.1), Interval(-0.1,0.1), Interval(-0.1,0.1));
    IVector y(Interval(-0.1,0.1), Interval(0.9, 1.1), Interval(-0.1,0.1));
    IVector z = x % y;
    //z.normalize();
    
    cout << z << " length: " << z.length() << endl;
    
    return 0;
}
