#include <valarray>

#include <cstdio>

#include "Rendezvous.h"
#include "Polynomial.h"
#include "Solve.h"

using namespace std;

Rendezvous::Rendezvous() : xt(0,0,0), vt(0,0,0), at(0,0,0),
xs(0,0,0), vs(0,0,0), as(0,0,0), last_t(0)
{ }

void Rendezvous::updateSource(
    const Vector & x, const Vector & v, const Vector & a) {
    xs = x;
    vs = v;
    as = a;
}

void Rendezvous::updateSource(
    double delta_t, const Vector & xs, const Vector & vs) {
    this->as = (vs - this->vs)/delta_t;
    this->xs = xs;
    this->vs = vs;
}

void Rendezvous::updateSource(double delta_t, const Vector & xs) {
    Vector vs = (xs - this->xs)/delta_t;
    this->as = (vs - this->vs)/delta_t;
    this->vs = vs;
    this->xs = xs;
}

void Rendezvous::updateTarget(
    const Vector & xt, const Vector & vt, const Vector & at) {
    this->xt=xt;
    this->vt=vt;
    this->at=at;
}

void Rendezvous::updateTarget(
    double delta_t, const Vector & xt, const Vector & vt) {
    this->at = (vt - this->vt)/delta_t;
    this->xt = xt;
    this->vt = vt;
}

void Rendezvous::updateTarget(double delta_t, const Vector & xt) {
    Vector vt = (xt - this->xt)/delta_t;
    this->at = (vt - this->vt)/delta_t;
    this->vt = vt;
    this->xt = xt;
}

// Vector Rendezvous::calculate() {
//
//     // Strategy: start with last time's solution. If that fails, we try
//     // to solve the order-2 equation. Using this solution as a start value,
//     // we get our hands on the order-4-equation another time. If this also
//     // fails we use the order-2-solution
//
//     Vector d = xt - xs;
//     float vtvt = vt*vt;
//     float vsvs = vs*vs;
//     float dvt = d*vt;
//     float dd = d*d;
//
//     float t;
//
//     float coeffs[5]; // highest order last
//     coeffs[4] = 0.25*(at*at);
//     coeffs[3] = at*vt;
//     coeffs[2] = d*at + vtvt - vsvs;
//     coeffs[1] = 2.0*dvt;
//     coeffs[0] = dd;
//
//     static int counter_a = 0, counter_a2 = 0, counter_b = 0;
//     if ((counter_a+counter_a2+counter_b) % 100 == 0)
//         printf("%d times newton using last solution, "
//                 "%d times newton using order-2-solution as start value, "
//                 "%d times order-2-solution\n",
//                 counter_a2, counter_a, counter_b);
//     
//     Polynomial<float> p(valarray<float>(coeffs, 5));
//     if (p.newton(last_t, &t, 0.1, 10) && t>=0) {
//         counter_a2++;
//         last_t = t;
//         return xt + t*(vt + t*0.5*at);
//     }
//     
//     // Ok, the last solution didn't give us a hint where the new solution is
//     // So we try another strategy: calculate an order-2-solution and use that
//     // as the basis for the next try
//     
//     {
//         float a = vtvt - vsvs;
//         float b = 2.0*(dvt);
//         float c = dd;
//         float t1, t2;
// 
//         if (Solve::order2(a,b,c, &t1, &t2)) {
//             if (t1 < 0 && t2 < 0) return xt;
//             t = max(t1, t2);
//         } else {
//             return xt;
//         }
//     }
//     
//     // we try a little harder this time
//     float new_t;
//     if (p.newton(t, &new_t, 0.1, 50) && new_t>=0) {
//         // We can return the order-4-solution
//         counter_a++;
//         last_t = new_t;
//         return xt + new_t*(vt + new_t*(0.5*at));
//     } else {
//         // Return order-2-solution
//         counter_b++;
//         last_t = t;
//         return t*vt + xt;
//     }
// }

// Vector Rendezvous::calculate() {
//     // Algorithm:
//     // Let "source" have position xs, velocity vs, acceleration as and
//     // let "target" have position xt, velocity vt, acceleration at
//     //
//     // 3rd-order taylor gives:
//     // xs(t) = xs + 1/2 vs t + 1/6 as t^2
//     // xt(t) = xt + 1/2 vt t + 1/6 at t^2
//     //
//     // ==> distance vector d(t) = (xs-xt) + 1/2 (vs-vt) t + 1/6 (as-at) t^2
//     //                         := dx + 1/2 dv t + 1/6 da t^2
//     // ==> squared distance |d(t)|^2 = d(t) * d(t)
//     //     = dx^2                       t^0 +
//     //       dx*dv                      t^1 +
//     //       (1/3 dx*da + 1/4 dv^2)     t^2 +
//     //       1/6 dv*da                  t^3 +
//     //       1/36 da*da                 t^4
//     //
//     // ==> first derivative (used for computing minima of squared distance)
//     //     = dx*dv                      t^0 +
//     //       (2/3 dx*da + 1/2 dv^2)     t^1 +
//     //       1/2 dv*da                  t^2 +
//     //       1/9 da*da                  t^3
//     //
//     // where '*' means the scalar (dot) product and x^2 means x*x
//     //
//     // Then compute the solutions of this third-order equation and return
//     // the one with the lowest |d(t)|^2
//     
//     Vector dx = xt - xs;
//     Vector dv = vt - vs;
//     Vector da = at - as;
//     
//     float t[3];
//     int n = Solve::order3(
//             da*da,
//             (dv*da)*3.0,
//             (dx*da + dv*dv)*2.0,
//             (dx*dv)*2.0,
//             t);
//     if (n == 0) return xt; // no solutions found (rarely the case)
//     
//     int best_sol = -1;
//     float best_d;
//     
//     for (int i=0; i<n; i++) {
//         float d;
//         if (t[i] >= 0 && (best_sol == -1 || t[i] < t[best_sol])) {
//             best_sol = i;
//             best_d = d;
//         }
//     }
//     
//     if (best_sol == -1) return xt;
//     else {
//         printf("We have a solution at %f\n", t[best_sol]);
//         return xt +
//                 (1.0/2.0)*t[best_sol] * vt +
//                 (1.0/6.0)*t[best_sol]*t[best_sol] * at;
//     }
// }

///////////////////////////////////////////////////////////////////////////////
// THIS IS THE BEST SOLUTION SO FAR ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
Vector Rendezvous::calculate() {
    Vector dx = xt - xs;
    float vt2 = vt*vt;
    float vs2 = vs*vs;
    float dxvt = dx*vt;
    float dx2 = dx*dx;

    float coeffs[5];
    coeffs[0] = -2.0*xt*xs + xt*xt;
    coeffs[1] = 2.0*(xt*vt - xs*vt - xs*vs);
    coeffs[2] = xt*at + vt2 - vs2 - xs*at - xs*as;
    coeffs[3] = vt*at - vs*as;
    coeffs[4] = (at*at - as*as) / 4.0;

    float t;
    Polynomial<float> p(valarray<float>(coeffs, 5));
    if (p.newton(last_t, &t, 0.1, 10) && t>=0) {
        last_t = t;
        return xt + t*(vt + t*0.5*at);
    }

    // Ok, the last solution didn't give us a hint where the new solution is
    // So we try another strategy: calculate an order-2-solution and use that
    // as the basis for the next try

    {
        float a = vt2 - vs2;
        float b = 2.0*(dxvt);
        float c = dx2;
        float t1, t2;

        if (Solve::order2(a,b,c, &t1, &t2)) {
            if (t1 < 0 && t2 < 0) return xt;
            t = max(t1, t2);
        } else {
            return xt;
        }
    }
    // Return order-2-solution
    last_t = t;
    return t*vt + xt;
}
*/

/*
Vector Rendezvous::calculate() {
    Vector x = xt - xs;
    Vector v = xt - vs;
    Vector a = at - as;

    float coeffs[4];
    coeffs[0] = 2.0f*(x*v);
    coeffs[1] = 2.0f*(x*a + v*v);
    coeffs[2] = 3.0f*(v*a);
    coeffs[3] = a*a;

    float t[4];
    int n = 1 + Solve::order3(coeffs[3], coeffs[2], coeffs[1], coeffs[0], 
&t[1]);
    t[0] = 0.0f;
    float min_t=t[0];
    Vector min_diff;
    float min_dist;
    for(int i=0; i<n; i++) {
        Vector diff = x+t[i]*(v+0.5*t[i]*a);
        float dist=diff.lengthSquare();
        if (i==0 || t[i]>0 && dist < min_dist) {
            min_dist = dist;
            min_diff = diff;
            min_t = t[i];
        }
    }

    last_t = min_t;
    //return min_diff + xt + min_t*(vt + 0.5*min_t*at);
    return xt + min_t*(vt + 0.5*min_t*at);
}
*/

/*
Vector Rendezvous::calculate() {
    Vector x = xt - xs;
    Vector v = vt - vs;
    Vector a = at - as;

    float coeffs[4];
    coeffs[0] = 2.0f*(x*v);
    coeffs[1] = 2.0f*(x*a + v*v);
    coeffs[2] = 3.0f*(v*a);
    coeffs[3] = a*a;

    //coeffs[0] = 6; coeffs[1]=-5; coeffs[2]=1; coeffs[3]=0;

    float t[4];
    t[0]=0;
    Polynomial<float> p(valarray<float>(coeffs, 4));
    int n=1;
    while(n<4 && p.newton(0, &t[n], 1e-2)) {
        printf("Found root %d at %f\n", n, t[n]);
        printf("value: %f\n", 
coeffs[0]+t[n]*(coeffs[1]+t[n]*(coeffs[2]+t[n]*(coeffs[3]))));
        p.divide(t[n]);
        ++n;
    }

    Vector min_diff;
    float min_dist;
    float min_t;
    for(int i=0; i<n; i++) {
        Vector diff = x+t[i]*(v+ 0.5*t[i]*a);
        float dist = diff.lengthSquare();

        if (i==0 || t[i]>=0 && dist < min_dist) {
            min_diff = diff;
            min_dist = dist;
            min_t = t[i];
        }
    }

    last_t = min_t;

    return min_diff + xt + min_t*(vt + 0.5*min_t*at);
}
*/

/*
Vector Rendezvous::calculate() {
    Vector dx = xt - xs;
    float vt2 = vt*vt;
    float vs2 = vs*vs;
    float dxvt = dx*vt;
    float dx2 = dx*dx;

    float coeffs[5];
    coeffs[0] = -2.0*xt*xs + xt*xt;
    coeffs[1] = 2.0*(xt*vt - xs*vt - xs*vs);
    coeffs[2] = xt*at + vt2 - vs2 - xs*at - xs*as;
    coeffs[3] = vt*at - vs*as;
    coeffs[4] = (at*at - as*as) / 4.0;

    float t[4];
    Polynomial<float> p(valarray<float>(coeffs, 5));
    int n=0;
    while(n<4 && p.newton(0, &t[n], 1e-1)) {
        if (!isnormal(t[n])) break;
        p.divide(t[n]);
        ++n;
    }

    float min_t=-1;
    for(int i=0; i<n; i++) {
        if (min_t < 0 || t[i]>0 && t[i] < min_t) {
            min_t = t[i];
        }
    }
    if (min_t < 0) min_t = 0;

    last_t = min_t;
    return xt + min_t*(vt + 0.5*min_t*at);
}
*/

Vector Rendezvous::calculate() {
    Vector at, vt;
    float eta = (xt-xs).length() / vel;
    if (eta < 4.0f) {
        at = this->at;
        vt = this->vt;
    } else if (eta < 8.0f) {
        at = sqrt((8*eta)/4) * this->at;
        vt = this->vt;
    } else if (eta < 16.0f) {
        at = Vector(0,0,0);
        vt = sqrt((16-eta)/8) * this->vt;
    } else {
        at = vt = Vector(0,0,0);
    }

    Vector x = xt - xs;
    Vector v = vt - vs;
    Vector a = at - as;


    /*
    ls_message("Rendezvous with:");
    ls_message("x: ");x.dump();
    ls_message("v: ");v.dump();
    ls_message("a: ");a.dump();
    */

    double coeffs[5];
    coeffs[0] = x*x;
    coeffs[1] = 2.0f*(v*x);
    coeffs[2] = a*x + v*v - vel*vel;
    coeffs[3] = a*v;
    coeffs[4] = 0.25f*a*a;

    //ls_message("Searching roots for %+f*t**4 %+f*t**3 + %+f*t**2 + %+f*t + %+f\n",
    //    coeffs[4], coeffs[3], coeffs[2], coeffs[1], coeffs[0]);

    //coeffs[0] = 6; coeffs[1]=-5; coeffs[2]=1; coeffs[3]=0;

    Polynomial<double> p(valarray<double>(coeffs, 5)), p_orig(p);
    double t[4];
    int n=0;
    while(n<4 && p.newton(last_t, &t[n], 1.0f, 40)) {
        if(!p_orig.newton(t[n], &t[n], 0.1f, 20)) break;
        //ls_warning("t[%d] = %f\n", n, t[n]);
        p.divide(t[n]);
        ++n;
    }

    float min_t;
    bool found=false;
    for(int i=0; i<n; i++) {
        if (t[i]>=0 && (t[i]<min_t || !found)) {
            min_t = t[i];
            found = true;
        }
    }
    if (!found) min_t = 0.0f;
    //ls_warning("Taking t=%f\n", min_t);
    if (min_t < 0) min_t = 0;
    /*
    if (min_t > 2) min_t = 2 + log(min_t-1);
    */

    last_t = min_t;

    return xt + min_t*(vt + 0.5*min_t*at);
}
