#include <iostream>
#include <iomanip>
#include "Quaternion.h"

using namespace std;

ostream & operator<< (ostream &os, const Quaternion & q) {
    os << setprecision(2) << fixed;
    os << "Q("  << q.real();
    os << " | " << q.imag()[0];
    os << ", " << q.imag()[1];
    os << ", " << q.imag()[2];
    os << ")";
}

ostream & operator<< (ostream &os, const Vector & v) {
    os << setprecision(2) << fixed;
    os << "V(" << v[0];
    os << ", " << v[1];
    os << ", " << v[2];
    os << ')';
}

#define PI 3.141592653589793238462

int main() {
    Quaternion rot1 = Quaternion::Rotation(Vector(0,1,0), PI/2);
    Vector v1(1,0,0);
    Vector v2 = rot1.rot(v1);
    cout << v1 << " rotated by " << rot1 << endl << " --> " << v2 << endl;
}
