#ifndef LINE_H
#define LINE_H

#include "Vector.h"

class Line {
    Vector p, v;
public:
    inline Line() { }
    inline Line(const Vector & p, const Vector & v) : p(p), v(v) { }
    // Pseudo-constructor
    inline static Line Between(const Vector & a, const Vector & b) {
        return Line(a, b-a);
    }
    
    inline Vector & P() { return p; }
    inline Vector & V() { return v; }
    inline const Vector & P() const { return p; }
    inline const Vector & V() const { return v; }
    
    inline float project(const Vector & x) const {
        return ((x-p) * v) / v.lengthSquare();
    }
};

#endif
