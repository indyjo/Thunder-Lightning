#include <iostream>
#include "Solve.h"

using namespace std;

int main() {
    cout << "Depressed cubic equation solver" << endl;
    cout << "-------------------------------" << endl;
    cout << "Solves y^3+A*y=B" << endl;
    cout << endl;
    
    cout << "A = ";
    float A;
    cin >> A;
    cout << endl;
    
    cout << "B = ";
    float B;
    cin >> B;
    cout << endl;
    
    float y[3];
    int n = Solve::order3depressed(A, B, y);
    cout << n << " solutions found." << endl;
    for (int i=0; i<n; i++) cout << "Solution " << i << ": " << y[i] << endl;
    
    cout << "Generic cubic equation solver" << endl;
    cout << "-------------------------------" << endl;
    cout << "Solves ay^3 + by^2+ cy + d = 0" << endl;
    cout << endl;
    float a,b,c,d;
    cout << "a = "; cin >> a;
    cout << "b = "; cin >> b;
    cout << "c = "; cin >> c;
    cout << "d = "; cin >> d;
    
    n = Solve::order3(a,b,c,d, y);
    cout << n << " solutions found." << endl;
    for (int i=0; i<n; i++) cout << "Solution " << i << ": " << y[i] << endl;
}
