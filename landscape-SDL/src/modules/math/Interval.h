#ifndef INTERVAL_H
#define INTERVAL_H

#include <cmath>
#include <stdexcept>
#include <ostream>

#include <landscape.h>

#ifndef PI
#define PI 3.141592653589793238462
#endif

template<class T> struct IntervalBase {
    T a,b;

    inline IntervalBase() { }
    inline IntervalBase(const T & val_a, const T & val_b) : a(val_a), b(val_b) { }
    inline IntervalBase(const T & val) : a(val), b(val) { }

    inline T length() const { return b-a; }
    
    inline friend bool operator> (const IntervalBase & X,
                                  const IntervalBase & Y)
    {
        return X.a > Y.b;
    }

    inline friend bool operator< (const IntervalBase & X,
                                  const IntervalBase & Y)
    {
        return X.b < Y.a;
    }
    
    inline friend bool intersect(const IntervalBase & X,
                                 const IntervalBase & Y)
    {
        return X.b >= Y.a && X.a <= Y.b;
    }

    inline friend IntervalBase intersection(const IntervalBase & X,
                                            const IntervalBase & Y)
            throw(std::invalid_argument)
    {
        IntervalBase result(std::max(X.a, Y.a), std::min(X.b, Y.b));
        if (result.length() < 0)
            throw std::invalid_argument("Intervals don't intersect.");
        return result;
    }
            
    inline friend const IntervalBase & operator+ (const IntervalBase & X) {
        return X;
    }
    
    inline friend IntervalBase operator- (const IntervalBase & X) {
        return IntervalBase(-X.b, -X.a);
    }
    
    inline friend IntervalBase operator+ (const IntervalBase & X,
                                          const IntervalBase & Y) {
        return IntervalBase(X.a+Y.a, X.b+Y.b);
    }
    inline friend const IntervalBase & operator+= (IntervalBase & X,
                                             const IntervalBase & Y) {
        X.a += Y.a;
        X.b += Y.b;
        return X;
    }
    
    inline friend IntervalBase operator- (const IntervalBase & X,
                                          const IntervalBase & Y) {
        return IntervalBase(X.a-Y.b, X.b-Y.a);
    }
    inline friend const IntervalBase & operator-= (IntervalBase & X,
                                             const IntervalBase & Y) {
        X.a -= Y.b;
        X.b -= Y.a;
        return X;
    }

    inline friend IntervalBase operator* (const IntervalBase & X,
                                          const IntervalBase & Y) {
        T aa = X.a*Y.a;
        T ab = X.a*Y.b;
        T ba = X.b*Y.a;
        T bb = X.b*Y.b;
        return IntervalBase(
                std::min(std::min(aa, ab), std::min(ba, bb)),
                std::max(std::max(aa, ab), std::max(ba, bb)));
    }
    inline friend const IntervalBase & operator*= (IntervalBase & X,
                                             const IntervalBase & Y) {
        return X = X * Y;
    }
    
    inline friend IntervalBase inv(const IntervalBase & X)
            throw(std::invalid_argument)
    {
        if (X.a > 0 || X.b < 0)
            return IntervalBase(1/X.b, 1/X.a);
        else
            throw std::invalid_argument("0 element of interval");
    }

    inline friend IntervalBase operator/ (const IntervalBase & X,
                                          const IntervalBase & Y)
            throw(std::invalid_argument)
    {
        return X * inv(Y);
    }
    inline friend const IntervalBase & operator /= (IntervalBase & X,
                                              const IntervalBase & Y)
            throw(std::invalid_argument)
    {
        return X = X / Y;
    }

    inline friend IntervalBase std::max(const IntervalBase & X,
                                   const IntervalBase & Y) {
        return IntervalBase(std::max(X.a, Y.a), std::max(X.b, Y.b));
    }
    inline friend IntervalBase std::min(const IntervalBase & X,
                                   const IntervalBase & Y) {
        return IntervalBase(std::min(X.a, Y.a), std::min(X.b, Y.b));
    }

    inline friend IntervalBase std::abs(const IntervalBase & X) {
        if (X.a<0 && X.b>0) {
            return IntervalBase(0, std::max(-X.a, X.b));
        }
        T abs_a = std::abs(X.a), abs_b = std::abs(X.b);
        if (abs_a <= abs_b)
            return IntervalBase(abs_a, abs_b);
        else
            return IntervalBase(abs_b, abs_a);
    }

    inline friend IntervalBase cos(const IntervalBase & X) {
        if (X.length() >= 2*PI)
            return IntervalBase(-1,1);
        int a = (int) floor(X.a * (1/PI));
        int b = (int) floor(X.b * (1/PI));
        int d = b-a;
        if (d >= 2) {
            return IntervalBase(-1,1);
        } else if (d == 1) {
            if (a & 1) // a is odd
                return IntervalBase(min(cos(X.a), cos(X.b)), 1);
            else       // a is even
                return IntervalBase(-1, max(cos(X.a), cos(X.b)));
        } else {
            if (a & 1) // a is odd
                return IntervalBase(cos(X.a), cos(X.b));
            else
                return IntervalBase(cos(X.b), cos(X.a));
        }
    }
    
    inline friend IntervalBase sin(const IntervalBase & X) {
        return cos(X - PI/2);
    }

    inline friend IntervalBase sqrt(const IntervalBase & X)
            throw(std::invalid_argument)
    {
        if (X.a >= 0) return IntervalBase(sqrt(X.a), sqrt(X.b));
        else throw std::invalid_argument("Interval must be >= 0.");
    }
    
    inline friend std::istream & operator>> (std::istream & is,
                                             IntervalBase & X)
    {
        //std::istream::sentry sntry(is);
        std::string s;
        char c;

        is >> s;
        if (!is || s != "Interval[") {
            is.setstate(std::ios_base::failbit);
            return is;
        }

        is >> X.a;
        is >> c; // skip ','
        is >> X.b;
        is >> c; // skip ']'
        return is;
    }

    inline friend std::ostream & operator<< (std::ostream & os,
                                             const IntervalBase & X)
    {
        return os << "Interval[ " << X.a << ", " << X.b << "]";
    }
};

template<class T>
IntervalBase<T> square(const IntervalBase<T> & x) {
    if (x.a > 0) return IntervalBase<T>(square(x.a), square(x.b));
    else if (x.b > 0) return IntervalBase<T>(0, square(std::max(-x.a, x.b)));
    else return IntervalBase<T>(square(x.b), square(x.a));
}

typedef IntervalBase<float> Interval;


#endif
