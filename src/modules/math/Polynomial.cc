#include <landscape.h>

#include "Polynomial.h"

using namespace std;

template<class T>
void Polynomial<T>::eval(T x, valarray<T> & y) {
    int n=a.size();
    int d=y.size();

    for(int i=0; i<d; i++) y[i] = 0;

    for(int i=n-1; i>=0; i--) {
        y[0] *= x;
        y[0] += a[i];

        for(int j=1; j<min(d,i+1); j++) {
            y[j] *= x;
            y[j] += y[j-1];
        }
    }
}


template<class T>
T Polynomial<T>::divide(T z) {
    T r = 0;
    for (int i= a.size()-1; i>=0; --i) {
        T a_i = a[i];
        a[i] = r;
        r = z*r+a_i;
    }
    if (a.size() > 1) a.pop_back();
    return r;
}

template<class T>
bool Polynomial<T>::newton( T x, T * x_res, T eps, int max_steps) {
    valarray<T> f(2);
    int count=0;
    T max_delta = 1e10;

    eval(x, f);
    //ls_message("x:%f f(x) = %f f'(x) = %f\n", x, f[0], f[1]);
    while(abs(f[0]) >= eps) {
        T delta = f[0]/f[1];
        if (abs(delta) > max_delta) return false;
        x -= delta;
        eval(x, f);
        //ls_message("x:%f f(x) = %f f'(x) = %f delta = %f\n", x, f[0], f[1], delta);
        if (++count > max_steps) return false;
    }
    *x_res = x;
    return true;
}

template<class T>
bool Polynomial<T>::aitken( T x0, T * x_res, T eps, int max_steps) {
    valarray<T> f(2);
    T x[3];
    x[0] = x0;
    for (int i=1; i<3; i++) {
        eval(x[i-1], f);
        x[i] = x[i-1] - f[0]/f[1];
    }
    
    double a = x[1] - x[0];
    double b = x[2] - x[1] - a;
    a*=a;
    
    x0 -= a/b;
    
    return newton(x0, x_res, eps, max_steps);
}

// Explicit instantiations
template class Polynomial<float>;
template class Polynomial<double>;
