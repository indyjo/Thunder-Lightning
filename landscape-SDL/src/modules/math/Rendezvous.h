#ifndef RENDEZVOUS_H
#define RENDEZVOUS_H

#include "Vector.h"

class Rendezvous {
    Vector xt, vt, at;
    Vector xs, vs, as;
    float last_t, vel;

public:
    Rendezvous();

    void setVelocity(float vel) { this->vel = vel; }

    void updateSource(const Vector & x, const Vector & v, const Vector & a);
    void updateSource(double delta_t, const Vector & xs, const Vector & vs);
    void updateSource(double delta_t, const Vector & xs);
    
    void updateTarget(const Vector & xt, const Vector & vt, const Vector & at);
    void updateTarget(double delta_t, const Vector & xt, const Vector & vt);
    void updateTarget(double delta_t, const Vector & xt);
    
    Vector calculate();
};

#endif
