#include <modules/math/Collide.h>
#include "LoDTerrain.h"

LoDQuad::CoordRel LoDQuad::getCoordRelX(float x)
{
    float vmin, vmax;
    
    vmin = vx[triangle[0].vertex[0]];
    vmax = vx[triangle[0].vertex[1]];
    
    if (x < vmin) return BELOW;
    else if (x > vmax) return ABOVE;
    else return INSIDE;
}

LoDQuad::CoordRel LoDQuad::getCoordRelZ(float z)
{
    float vmin, vmax;
    
    vmin = vz[triangle[0].vertex[0]];
    vmax = vz[triangle[0].vertex[1]];
    
    if (z < vmin) return BELOW;
    else if (z > vmax) return ABOVE;
    else return INSIDE;
}

float LoDQuad::getHeightAt(float x, float z)
{
    float height;
    getHeightAtTriangle(&triangle[0], x, z, &height)
            || getHeightAtTriangle(&triangle[1], x, z, &height);
    return height;
}


#define VSCALAR(x1,y1,x2,y2) ((x1)*(x2)+(y1)*(y2))
// Returns false if (x/z) is not inside
// Our triangle looks like this:
//             2 p2
//        v2,-'\
//      2,-'    \ 1 v1
//    ,-'        \
// 0 -------------- 1
// p0      0 v0     p1

bool LoDQuad::getHeightAtTriangle(LoDTriangle *tri, float x, float z, float *height)
{
    float px[3], py[3], pz[3]; // the triangle corners
    float vx[3], vz[3];        // Direction vectors of the tri edges
    float nx[3], nz[3];        // Normal vectors of the tri edges
    float d[3];                // Distance of (x,z) to the according edge
    float dp[3];               // Distance of a corner point to the opposing edge
    bool all_positive;
    // Copy the triangle corner points
    for(int i=0; i<3; i++) {
        px[i]=this->vx[tri->vertex[i]];
        py[i]=this->vy[tri->vertex[i]];
        pz[i]=this->vz[tri->vertex[i]];
        //ls_warning("px[%d] = %f\tpz[%d] = %f\n", i, px[i], i, pz[i]);
    }
    
    // Calculate the edge direction vectors
    for(int i=0; i<3; i++) {
        vx[i] = px[(i+1)%3] - px[i];
        vz[i] = pz[(i+1)%3] - pz[i];
        //ls_message("vx[%d] = %f\tvz[%d] = %f\n", i, vx[i], i, vz[i]);
    }
    
    // Calculate the edge normals. They should point towards the triangle.
    // Our triangles are counter-clockwise (LEFT, RIGHT, TOP)
    // So our normals should be the direction vectors rotated counter-clockwise
    // By 90 degrees
    for (int i=0; i<3; i++) {
        nx[i] = -vz[i];
        nz[i] =  vx[i];
    }
    
    // Calculate the distance between (x,z) to every edge. This need not be
    // normalized because at this time we are just interested in the sign of
    // the distance.
    all_positive=true;
    for (int i=0; i<3; i++) {
        d[i] = VSCALAR(nx[i], nz[i], x-px[i], z-pz[i]);
        if (d[i]<0) all_positive=false;
    }
    
    //ls_error("d: [%f, %f, %f]\n", d[0], d[1], d[2]);
    
    // If not all edge distances are positive, we are done, because the point
    // Is not inside this triangle
    if (!all_positive) return false;
    
    // Now, if the triangle has children, pass the test to them. Else, we have
    // to calculate the interpolated y value (height) of the tri at (x,z)
    if (tri->flags & TFLAG_HAS_CHILDREN) {
        return getHeightAtTriangle(tri->child[0], x, z, height)
                || getHeightAtTriangle(tri->child[1], x, z, height);
    } else {
        dp[1] = VSCALAR(nx[2], nz[2], px[1]-px[2], pz[1]-pz[2]);
        dp[0] = VSCALAR(nx[1], nz[1], px[0]-px[2], pz[0]-pz[2]);
        *height = py[2]
                + ((py[1] - py[2]) * d[2]) / dp[1]
                + ((py[0] - py[2]) * d[1]) / dp[0];
        return true;
    }
}


// #define EPSILON 0.01
// #define MAX_INTERVAL_SQUARE 100000.0
// bool LoDQuad::lineCollides(Vector a, Vector b, Vector * x, LoDTriangle * tri)
// {
//     float h_a = getHeightAt(a[0], a[2]);
//     float h_b = getHeightAt(b[0], b[2]);
//     
//     if (h_a >= a[1]) {
//         *x=a;
//         return true;
//     }
//     
//     if ((a-b).lengthSquare() > MAX_INTERVAL_SQUARE) {
//         return lineCollides(a, (a+b)/2.0, x) || lineCollides((a+b)/2.0, b, x);
//     }
//      
//     if (h_a < a[1] && h_b < b[1]) return false;
//     
//     float delta = (a-b).length();
//     while(delta >= EPSILON) {
//         Vector m = (a + b) / 2;
//         float h_m = getHeightAt(m[0], m[2]);
//         if (h_a >= a[1]) {
//             if (h_m >= m[1]) {
//                 a = m;
//                 h_a = h_m;
//             } else {
//                 b = m;
//                 h_b = h_m;
//             }
//         } else {
//             if (h_m >= m[1]) {
//                 b = m;
//                 h_b = h_m;
//             } else {
//                 a = m;
//                 h_a = h_m;
//             }
//         }
//         delta /= 2;
//     }
//     
//     *x = a;
//     return true;
// }

bool LoDQuad::lineCollides(Vector a, Vector b, float * t, LoDTriangle * tri)
{
    float t0, t1;
    
    //ls_message("Checking for collision in tri %p from ", tri);
    //a.dump();
    //ls_message("to ");
    //b.dump();
    
    // bounding sphere test
    if (!Collide::points((a+b)/2, tri->bs_center,
            ((a-b)/2).length(), tri->radius)) return false;
    if (!Collide::movingPoints(
            Line::Between(a,b),
            Line::Between(tri->bs_center, tri->bs_center),
            0.0, tri->radius,
            &t0, &t1)) {
        return false;
    }
    if ( t1 < 0  || t0 > 1) return false;
    *t = t0;
    
    if (tri->flags & TFLAG_HAS_CHILDREN) {
        /*
        t0 = std::max(0.0f, t0);
        t1 = std::min(1.0f, t1);
        */
        Vector p0( vx[tri->vertex[0]], vy[tri->vertex[0]], vz[tri->vertex[0]]);
        Vector p1( vx[tri->vertex[1]], vy[tri->vertex[1]], vz[tri->vertex[1]]);
        // using the line's direction we calculate which child triangle we have
        // to check first. If the lines's direction is from left to right,
        // it's the left triangle, if it's from right to left, it's the right
        // one.
        bool left_first = (p1-p0) * (b-a) > 0.0f;
        /*
        bool c0, c1;
        c0 = lineCollides(a, b, &t0, tri->child[0]);
        c1 = lineCollides(a, b, &t1, tri->child[1]);
        if (c0 && c1) {
            *t = std::min(t0, t1);
            return true;
        } else if (c0) {
            *t = t0;
            return true;
        } else if (c1) {
            *t = t1;
            return true;
        } else return false;
        */
        if (lineCollides(a,b, t, tri->child[left_first?0:1])) {
            return true;
        } else if (lineCollides(a,b, t, tri->child[left_first?1:0])) {
            return true;
        } else return false;
    }
    
    // plane collision test
    Vector p0( vx[tri->vertex[0]], vy[tri->vertex[0]], vz[tri->vertex[0]]);
    Vector p1( vx[tri->vertex[1]], vy[tri->vertex[1]], vz[tri->vertex[1]]);
    Vector p2( vx[tri->vertex[2]], vy[tri->vertex[2]], vz[tri->vertex[2]]);
    if (!Collide::lineOnPlane(Line::Between(a,b), Plane(p0,p2,p1), t)) {
        return false;
    }
    // t has to be between 0 and 1, otherwise found point is not on [ab]
    if (*t < 0 || *t > 1) return false;
    
    // triangle intersection test via conversion into baricentric coordinates
    Vector x = a + (*t) * (b-a) - p2;
    Vector d1 = p0 - p2;
    Vector d2 = p1 - p2;
    float ta = (x*d1) / d1.lengthSquare();
    float tb = (x*d2) / d2.lengthSquare();
    if ( ta>=0 && tb>=0 && ta+tb<=1 ) {
        //ls_warning("hit!");
        //x.dump();
        return true;
    }
    else return false;
}

