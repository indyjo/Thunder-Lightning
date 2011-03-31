#ifndef BALLISTIC_H
#define BALLISTIC_H

namespace Ballistic {

// Calculates solutions for shooting a target at (dx,dy) from (0,0) with
// a given start velocity v0
// dx : the horizontal distance to the target (must be >0)
// dy : the vertical distance (signed, >0 means above)
// v0 : the start velocity of the projectile
// g  : the gravity (earth = -9.81f)
// vx0, vy0 : address of the first possible solution
// vx1, vy1 : address of the second possible solution
// returns : number of found solutions (0 <= n <= 2), *vx1 >= *vx2
int solve(float dx, float dy, float v0, float g,
          float * vx0, float * vy0,
          float * vx1, float * vy1);

}

#endif
