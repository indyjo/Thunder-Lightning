#include <vector>
#include "BoundingGeometry.h"

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

std::ostream & operator<< (std::ostream & out, const BoundingGeometry & bg) {
    out << "Geometry( " << bg.bounding_radius << std::endl;
    out << "(" << std::endl;
    typedef std::vector<std::string>::const_iterator It;
    for (It i = bg.domains.begin(); i != bg.domains.end(); i++)
        out << *i << std::endl;
    out << ")" << std::endl;

    out << "( ";
    for(int i=0; i<bg.transform_parents.size(); i++)
        out << bg.transform_parents[i] << " ";
    out << ")" << std::endl;

    out << bg.root << std::endl;
    out << ")";

    return out;
}

std::istream & operator>> (std::istream & in, BoundingGeometry & bg) {
    std::istream::sentry sentry(in);

    std::string s;

    expect(in, "Geometry(");

    in >> bg.bounding_radius;

    in >> s; // read '('

    while(true) {
        in >> s;
        if (s == ")") break;
        bg.domains.push_back(s);
    }

    in >> s; // read '('
    while(true) {
        in >> s;
        if (s == ")") break;
        bg.transform_parents.push_back(atoi(s.c_str()));
    }

    in >> bg.root;
    in >> s; // read ')'

    check(in);
    return in;
}
	
	
} // namespace Collide

