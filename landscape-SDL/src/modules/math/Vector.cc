#include <iostream>
#include <string>
#include <sstream>

#include "Vector.h"
#include "Interval.h"

template<int N, class T>
std::istream & operator>> (std::istream & is,
                           XVector<N,T> & v)
{
    std::istream::sentry sentry(is);

    std::ostringstream ost;
    ost << "Vector" << N << "(";

    std::string s;
    is >> s;
    if (!is || s != ost.str()) {
        is.setstate(std::ios_base::failbit);
        return is;
    }

    for (int i=0; i<N; i++) {
        char c;
        is >> v[i] >> c;
    }

    if (!is) is.setstate(std::ios_base::failbit);

    return is;
}


template<int N, class T>
std::ostream & operator<< (std::ostream & os,
                           const XVector<N,T> & v)
{
    os << "Vector" << N  << "( ";
    for(int i=0; i<N; i++)
        os << v[i] << ((i==N-1)?")":", ");
    return os;
}

// Explicit instantiations
template std::istream & operator>> (std::istream & is,
                                    XVector<3,float> & v);
template std::istream & operator>> (std::istream & is,
                                    XVector<3,Interval> & v);
template std::ostream & operator<< (std::ostream & os,
                                    const XVector<3,float> & v);
template std::ostream & operator<< (std::ostream & os,
                                    const XVector<3,Interval> & v);
