#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <tnl.h>
#include <valarray>
#include <vector>

template<class T>
class Polynomial {
    std::vector<T> a;

public:
    Polynomial(const std::valarray<T> & coeffs) {
        a.resize(coeffs.size());
        for(int i=0; i<coeffs.size(); i++) {
            a[i] = coeffs[i];
        }
    }

    void eval(T x, std::valarray<T> & y);

    T divide(T z);

    // tries to find an x with f(x)=0, starting at x0
    bool newton( T x0, T * x, T eps, int max_steps = 10);
    bool aitken( T x0, T * x, T eps, int max_steps = 10);

    inline T operator() (T x) {
        std::valarray<T> y(1);
        eval(x, y);
        return y[0];
    }
};

#endif
