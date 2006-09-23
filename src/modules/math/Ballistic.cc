#include "Solve.h"
#include "Ballistic.h"

namespace Ballistic {

int solve(float dx, float dy, float v0, float g,
          float * vx0, float * vy0,
          float * vx1, float * vy1)
{
    float v0_2 = v0*v0;
    float dx_2 = dx*dx;
    float dy_2 = dy*dy;
    float g_2 = g*g;
    
    float a = dx_2+dy_2;
    float b = -dx_2*(g*dy + v0_2);
    float c = 0.25f * g_2 * dx_2*dx_2;
    
    float u[2];
    if (!Solve::order2(a,b,c,&u[0],&u[1])) return 0;
    
    int n;
    
    if (u[0]>0) {
        if (u[1]>0) {
            n = 2;
        } else {
            n = 1;
        }
    } else {
        if (u[1]>0) {
            n = 1;
            u[0] = u[1];
        } else {
            return 0;
        }
    }
    
    *vx0 = sqrt(u[0]);
    *vy0 = (2*dy*u[0]-g*dx_2) / (2*dx*(*vx0));
    
    if (n==2) {
        *vx1 = sqrt(u[1]);
        *vy1 = (2*dy*u[1]-g*dx_2) / (2*dx*(*vx1));
    }
    
    return n;
}

}
