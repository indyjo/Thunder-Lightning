#include <iostream>

#include <landscape.h>

#include <modules/math/Interval.h>
#include "Primitive.h"

using namespace std;
using namespace Collide;

int main() {
    typedef XVector<3,Interval> IVector;
    IVector pos1 = IVector(
        Interval(324.6,324.6),
        Interval(731.9,731.9),
        Interval(931.8,931.9));
    IVector pos2 = IVector(
        Interval(320.4,325.4),
        Interval(726.5,735.9),
        Interval(925.0,941.6));
    IVector orient[3] = {
        IVector(
            Interval(-0.9, -0.9),
            Interval(-0.4, -0.4),
            Interval(0.1, 0.1)),
        IVector(
            Interval(-0.4, -0.4),
            Interval(0.9, 0.9),
            Interval(-0.1, -0.1)),
        IVector(
            Interval(-0.0, -0.0),
            Interval(-0.1, -0.1),
            Interval(-1.0, -1.0))};
    BoundingBox box = {
        Vector(-0.014050, 0.166725, 0.692275),
        {10.000500, 2.500125, 6.340315}};
    Hints hints;


    cout << "Collision test." << endl;
    if (intersectBoxSphere(box, pos1, orient, 0.01, pos2,hints))
        cout << "Collision!" << endl;
    else cout << "No collision" << endl;
}
