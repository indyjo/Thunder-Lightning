#include <string>
#include "Interval.h"


using namespace std;

template<class T>
IntervalBase<T> intersection(const IntervalBase<T> & X,
							 const IntervalBase<T> & Y)
            throw(std::invalid_argument)
{
    IntervalBase<T> result(std::max(X.a, Y.a), std::min(X.b, Y.b));
    if (result.length() < 0)
        throw std::invalid_argument("Intervals don't intersect.");
    return result;
}

template<class T>
IntervalBase<T> operator* (const IntervalBase<T> & X,
                           const IntervalBase<T> & Y) {
    T aa = X.a*Y.a;
    T ab = X.a*Y.b;
    T ba = X.b*Y.a;
    T bb = X.b*Y.b;
    return IntervalBase<T>(
            std::min(std::min(aa, ab), std::min(ba, bb)),
            std::max(std::max(aa, ab), std::max(ba, bb)));
}

template<class T>
IntervalBase<T> inv(const IntervalBase<T> & X)
        throw(std::invalid_argument)
{
    if (X.a > 0 || X.b < 0)
        return IntervalBase<T>(1/X.b, 1/X.a);
    else
        throw std::invalid_argument("0 element of interval");
}

template<class T>
IntervalBase<T> std::abs(const IntervalBase<T> & X) {
    if (X.a<0 && X.b>0) {
        return IntervalBase<T>(0, std::max(-X.a, X.b));
    }
    T abs_a = std::abs(X.a), abs_b = std::abs(X.b);
    if (abs_a <= abs_b)
        return IntervalBase<T>(abs_a, abs_b);
    else
        return IntervalBase<T>(abs_b, abs_a);
}

#ifndef PI
#define PI 3.141592653589793238462
#endif

template<class T>
IntervalBase<T> std::cos(const IntervalBase<T> & X) {
    if (X.length() >= 2*PI)
        return IntervalBase<T>(-1,1);
    int a = (int) floor(X.a * (1/PI));
    int b = (int) floor(X.b * (1/PI));
    int d = b-a;
    if (d >= 2) {
        return IntervalBase<T>(-1,1);
    } else if (d == 1) {
        if (a & 1) // a is odd
            return IntervalBase<T>(min(cos(X.a), cos(X.b)), 1);
        else       // a is even
            return IntervalBase<T>(-1, max(cos(X.a), cos(X.b)));
    } else {
        if (a & 1) // a is odd
            return IntervalBase<T>(cos(X.a), cos(X.b));
        else
            return IntervalBase<T>(cos(X.b), cos(X.a));
    }
}


template<class T>
IntervalBase<T> std::sin(const IntervalBase<T> & X) {
    return cos(X - PI/2);
}


template<class T>
std::istream & operator>> (std::istream & is,
                           IntervalBase<T> & X)
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

template<class T>
std::ostream & operator<< (std::ostream & os,
                           const IntervalBase<T> & X)
{
    return os << "Interval[ " << X.a << ", " << X.b << "]";
}

template<class T>
IntervalBase<T> square(const IntervalBase<T> & x) {
    if (x.a > 0) return IntervalBase<T>(square(x.a), square(x.b));
    else if (x.b > 0) return IntervalBase<T>(0, square(std::max(-x.a, x.b)));
    else return IntervalBase<T>(square(x.b), square(x.a));
}



// explicit instantiations
template
IntervalBase<float> intersection(const IntervalBase<float> & X,
							 const IntervalBase<float> & Y);
template
IntervalBase<float> operator* (const IntervalBase<float> & X,
                           const IntervalBase<float> & Y);
template
IntervalBase<float> inv(const IntervalBase<float> & X)
        throw(std::invalid_argument);
template
IntervalBase<float> square(const IntervalBase<float> & x);

namespace std {
	template
	IntervalBase<float> abs<>(const IntervalBase<float> & X);
	template
	IntervalBase<float> cos<>(const IntervalBase<float> & X);
	template
	IntervalBase<float> sin<>(const IntervalBase<float> & X);
}

template
std::istream & operator>> (std::istream & is,
                           IntervalBase<float> & X);
template
std::ostream & operator<< (std::ostream & os,
                           const IntervalBase<float> & X);
