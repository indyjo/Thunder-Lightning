#include <cstdlib>
#include <iostream>
#include <iomanip>
#include "Interval.h"

using namespace std;

int main(int argc, char ** argv) {
    if (argc != 3) {
        cerr << "Usage: test_interval a b" << endl;
        return -1;
    }
    
    float a = atof(argv[1]);
    float b = atof(argv[2]);
    Interval X(a,b);
    
    cout << fixed << setprecision(3);
    cout << "cos( " << X << " ) = " << cos(X) << endl;
    cout << "sin( " << X << " ) = " << sin(X) << endl;
    try {
        cout << "sqrt( " << X << " ) = " << sqrt(X) << endl;
    } catch (invalid_argument e) {
        cout << "(error: '" << e.what() << "')" << endl;
    }
    cout << "abs( " << X << " ) = " << abs(X) << endl;
    cout << endl;
    
    return 0;
}
