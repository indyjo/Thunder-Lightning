#include <iostream>
#include <iomanip>
#include "Quaternion.h"
#include "Interval.h"

using namespace std;

typedef XQuaternion<Interval> IQuaternion;
typedef XVector<3, Interval> IVector;

ostream & operator<< (ostream &os, const IQuaternion & q) {
    os << setprecision(2) << fixed;
    os << "Q("  << q.real();
    os << " | " << q.imag()[0];
    os << ", " << q.imag()[1];
    os << ", " << q.imag()[2];
    os << ")";
}

ostream & operator<< (ostream &os, const IVector & v) {
    os << setprecision(2) << fixed;
    os << "V(" << v[0];
    os << ", " << v[1];
    os << ", " << v[2];
    os << ')';
}

#define PI 3.141592653589793238462

int main() {
	{
    IQuaternion rot1( Interval(0.79, 0.81),
    				  Interval(0.01, 0.03),
    				  Interval(0.58, 0.60),
    				  Interval(-0.09, -0.07));
    Vector v1(-0.04,   0.13,   0.00);
    IVector v2 = rot1.rot((IVector &) v1);
    cout << v1 << " rotated by " << rot1 << endl << " --> " << v2 << endl;
	}
	cout << endl << endl;
	{
    IQuaternion rot1( Interval( 0.105618,  0.105681),
    				  Interval(-0.782585, -0.782546),
    				  Interval(-0.477946, -0.477878),
    				  Interval( 0.384740,  0.384750));
    Vector v1(0.47,  -0.15,   0.87);
    IVector v2 = rot1.rot(IVector(v1));
    cout << v1 << " rotated by " << rot1 << endl << " --> " << v2 << endl;
	}
	cout << endl << endl;
	{
    IQuaternion rot1( Interval( 0.105618,  0.105681),
    				  Interval(-0.782585, -0.782546),
    				  Interval(-0.477946, -0.477878),
    				  Interval( 0.384740,  0.384750));
    IVector v1(Interval(0.47,0.48),
              Interval(-0.16,-0.15),
              Interval(0.87, 0.88));
    IVector v2 = rot1.rot(v1);
    cout << v1 << " rotated by " << rot1 << endl << " --> " << v2 << endl;
	}
}
