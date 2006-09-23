#include "Vector.h"

class Plane {
    Vector n; // normal vector
    float  a; // constant
    
public:
    inline Plane() { }
    inline Plane(const float * pl) : n((float(&)[3]) *pl), a(pl[3]) { }
    inline Plane(const Vector & n, float a = 0) : n(n), a(a) { }
    inline Plane(const Vector & p, const Vector & norm) {
        n = norm;
        a = - p*n;
    }
    // construct a plane from a triangle
    inline Plane(const Vector & p0, const Vector & p1, const Vector & p2) {
        n = (p1-p0)%(p2-p0);
        a = -n*p0;
    }
    
    inline const Vector & N() const { return n; }
    inline float          A() const { return a; }
    inline Vector & N() { return n; }
    inline float  & A() { return a; }
    
    // "Hesse-Normal-Form": n.length() == 1 && a<=0
    inline Plane & hnf() {
        float l = n.length();
        if (a>0) l = -l;
        n /= l;
        a /= l;
        return *this;
    }
    
    inline float d(const Vector & v) const { return v*n + a; }
    
    inline Vector & project(Vector & v) const { return v -= d(v) * n; }
    
    inline float operator[] (int i) const { return i<3?n[i]:a; }
    inline float & operator[] (int i) { return i<3?n[i]:a; }
};
