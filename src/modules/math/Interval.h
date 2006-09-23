#ifndef INTERVAL_H
#define INTERVAL_H

#include <cmath>
#include <stdexcept>
#include <ostream>

#include <landscape.h>

template<class T> struct IntervalBase;

template<class T>
IntervalBase<T> intersection(const IntervalBase<T> & X,
                             const IntervalBase<T> & Y)
        throw(std::invalid_argument);
template<class T>
IntervalBase<T> operator* (const IntervalBase<T> & X,
                           const IntervalBase<T> & Y);
template<class T>
IntervalBase<T> inv(const IntervalBase<T> & X)
        throw(std::invalid_argument);

namespace std {
	template<class T>
	IntervalBase<T> abs(const IntervalBase<T> & X);
	template<class T>
    IntervalBase<T> cos(const IntervalBase<T> & X);
	template<class T>
    IntervalBase<T> sin(const IntervalBase<T> & X);
	template<class T>
	IntervalBase<T> min(const IntervalBase<T> &, const IntervalBase<T>);
	template<class T>
	IntervalBase<T> max(const IntervalBase<T> &, const IntervalBase<T>);
}

template<class T>
std::istream & operator>> (std::istream & is,
                           IntervalBase<T> & X);
template<class T>
std::ostream & operator<< (std::ostream & os,
                           const IntervalBase<T> & X);
							 
template<class T>
IntervalBase<T> square(const IntervalBase<T> & x);



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

    friend IntervalBase intersection<>(const IntervalBase & X,
                                     const IntervalBase & Y)
            throw(std::invalid_argument);
            
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

    friend IntervalBase operator* <>(const IntervalBase & X,
                                     const IntervalBase & Y);
    inline friend const IntervalBase & operator*= (IntervalBase & X,
                                             const IntervalBase & Y) {
        return X = X * Y;
    }
    
    friend IntervalBase inv<>(const IntervalBase & X)
        throw(std::invalid_argument);

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


    inline friend IntervalBase sqrt(const IntervalBase & X)
            throw(std::invalid_argument)
    {
        if (X.a >= 0) return IntervalBase(sqrt(X.a), sqrt(X.b));
        else throw std::invalid_argument("Interval must be >= 0.");
    }
    
    friend std::istream & operator>> <>(std::istream & is,
                                       IntervalBase & X);

    friend std::ostream & operator<< <>(std::ostream & os,
                                       const IntervalBase & X);

};


namespace std{
	template<class T>
	IntervalBase<T> max(const IntervalBase<T> & X, const IntervalBase<T> & Y)
	{
		return IntervalBase<T>(std::max(X.a, Y.a), std::max(X.b, Y.b));
	}

	template<class T>
	IntervalBase<T> min(const IntervalBase<T> & X, const IntervalBase<T> & Y)
	{
		return IntervalBase<T>(std::min(X.a, Y.a), std::min(X.b, Y.b));
	}
}

typedef IntervalBase<float> Interval;


#endif
