#include "Ballistic.h"
#include <cmath>

#include <iostream>

using namespace std;

int main() {
    float dx = 4000.0f;
    float dy = 0.0f;
    float v0 = 300.0f;
    float vx[2], vy[2];
    int n = Ballistic::solve(dx,dy,v0,-9.81f,&vx[0], &vy[0], &vx[1], &vy[1]);
    
    cout << "dx = " << dx << " dy = " << dy << " v0 = " << v0 << " ";
    cout << "Found " << n << " solutions:" << endl;
    for(int i=0; i<n; i++) {
        cout << vx[i] << " " << vy[i] << " -> v= ";
        cout << sqrt(vx[i]*vx[i] + vy[i]*vy[i]) << ", alpha= ";
        cout << atan(vx[i]/vy[i]) * 180.0f / 3.141593f << "°" << endl;
    }
    cout << endl;
    
    float delta_t = 0.5f;
    
    for(int i=0; i<n; i++) {
        cout << "Solution " << i+1 << ":" << endl;
        float x=0, y=0;
        float t=0;
        vy[i]-=0.5f*9.81f*delta_t;
        while (x <= dx) {
            t += delta_t;
            x+=vx[i]*delta_t;
            y+=vy[i]*delta_t;
            vy[i] -= 9.81f*delta_t;
            
            cout << "  t= " << t << ":  " << x << " " << y << endl;
        }
    }
}
    
    
