#include <iostream>
#include "BoundingBox.h"

using namespace std;
using namespace Collide;

int main() {
    BoundingGeometry bg;
    cin >> bg;
    if (!cin) cerr << "An error occurred." << endl;
    
    cout << bg;
}
