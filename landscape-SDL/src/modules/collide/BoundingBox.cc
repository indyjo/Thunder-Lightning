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


void BoundingNode::cleanup() {
    switch(type) {
    case BoundingNode::NONE: break;
    case BoundingNode::LEAF:
        delete [] data.leaf.vertices;
        break;
    case BoundingNode::INNER:
        delete data.inner.children[0];
        delete data.inner.children[1];
        break;
    case BoundingNode::NEWDOMAIN:
        delete data.domain.child;
        break;
    case BoundingNode::TRANSFORM:
        delete data.transform.child;
        break;
    }
    type = BoundingNode::NONE;
}


std::ostream & operator<< (std::ostream & out, const BoundingNode & bn) {
    out << "Node( " << bn.box << " ";
    switch(bn.type) {
    case BoundingNode::NONE:
        out << "N ";
        break;
    case BoundingNode::LEAF:
        out << "L ";
        out << bn.data.leaf.n_triangles << " ";
        for(int i=0; i<bn.data.leaf.n_triangles * 3; i++) {
            out << bn.data.leaf.vertices[i];
            if(i % 3 == 0) out << std::endl;
            else out << " ";
        }
        break;
    case BoundingNode::INNER:
        out << "I" << std::endl;
        out << *bn.data.inner.children[0] << std:: endl;
        out << *bn.data.inner.children[1] << std:: endl;
        break;
    case BoundingNode::NEWDOMAIN:
        out << "D " << bn.data.domain.domain_id << std::endl;
        out << *bn.data.domain.child << std::endl;
        break;
    case BoundingNode::TRANSFORM:
        out << "T " << bn.data.transform.transform_id << std::endl;
        out << *bn.data.transform.child << std::endl;
        break;
    }
    out << ")";

    return out;
}

std::istream & operator>> (std::istream & in, BoundingNode & bn) {
    std::istream::sentry sentry(in);

    bn.cleanup();

    std::string s;
    char c;
    int n;

    expect(in, "Node(");

    check(in);
    in >> bn.box;
    check(in);

    in >> s;
    c = s[0];
    switch(c) {
    case 'N':
        break;
    case 'L':
        bn.type = BoundingNode::LEAF;
        in >> n;
        bn.data.leaf.n_triangles = n;
        bn.data.leaf.vertices = new Vector[n*3];
        for(int i=0; i<n*3; i++)
            in >> bn.data.leaf.vertices[i];
        break;
    case 'I':
        bn.type = BoundingNode::INNER;
        bn.data.inner.children[0] = new BoundingNode;
        bn.data.inner.children[1] = new BoundingNode;
        in >> *bn.data.inner.children[0] >> *bn.data.inner.children[1];
        break;
    case 'D':
        bn.type = BoundingNode::NEWDOMAIN;
        in >> bn.data.domain.domain_id;
        bn.data.domain.child = new BoundingNode;
        in >> *bn.data.domain.child;
        break;
    case 'T':
        bn.type = BoundingNode::TRANSFORM;
        in >> bn.data.transform.transform_id;
        bn.data.transform.child = new BoundingNode;
        in >> *bn.data.transform.child;
        break;
    default:
        std::cerr << "Bad node type: " << s << std::endl;
        in.setstate(std::ios_base::failbit);
        return in;
    }

    in >> c;
    check(in);
    return in;
}


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
