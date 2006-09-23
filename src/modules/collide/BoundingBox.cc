#include <modules/math/Interval.h>
#include <cmath>
#include "BoundingBox.h"

#define expect(in, what) {                                  \
    std::string s;                                          \
    in >> s;                                                \
    if (!in || s != what) {                                 \
        std::cerr << "At Line " << __LINE__ << ": Expected " << what<< std::endl;  \
        std::cerr << "    but got " << s << std::endl;      \
        in.setstate(std::ios_base::failbit);                \
        return in;                                          \
    }}

#define check(in)
//#define check(in) if (!in) std::cerr << "Bad stream at " << __LINE__ << std::endl;

namespace Collide {

std::ostream & operator<< (std::ostream & out, const BoundingBox & bb) {
    out << "Box( " << bb.pos << ' ';
    for(int i=0; i<3; i++) out << bb.dim[i] << ' ';
    out << ')';

    return out;
}

std::istream & operator>> (std::istream & in, BoundingBox & bb) {
    std::istream::sentry sentry(in);

    expect(in, "Box(");

    check(in);
    in >> bb.pos;
    check(in);
    char c;
    for(int i=0; i<3; i++) in >> bb.dim[i];

    in >> c;

    check(in);
    return in;
}



} // namespace Collide
