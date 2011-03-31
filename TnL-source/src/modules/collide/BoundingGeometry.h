#ifndef COLLIDE_BOUNDINGGEOMETRY_H
#define COLLIDE_BOUNDINGGEOMETRY_H

#include <modules/math/Vector.h>
#include "BoundingNode.h"

namespace Collide {

class  BoundingGeometry : virtual public Object {
    float bounding_radius;
    BoundingNode root;
    std::vector<std::string> domains;
    std::vector<int> transform_parents;
public:
    inline BoundingGeometry() : bounding_radius(0)
    { }

    inline BoundingGeometry(int d, int p) {
        while(d--) domains.push_back("default");
        while(p--) transform_parents.push_back(0);
    }

    inline float getBoundingRadius() const { return bounding_radius; }
    inline void setBoundingRadius(float r) { bounding_radius = r; }
    inline const BoundingNode * getRootNode() const { return &root; }

    inline int getNumOfDomains() const { return domains.size(); }
    inline const std::string & getDomainName(int i) const { return domains[i]; }

    inline int addDomainName(const std::string & s) {
        domains.push_back(s);
        return domains.size()-1;
    }

    inline int getNumOfTransforms() const { return transform_parents.size(); }
    inline int getParentOfTransform(int i) const { return transform_parents[i]; }

    friend std::ostream & operator<< (std::ostream & out, const BoundingGeometry & bg);
    friend std::istream & operator>> (std::istream & in, BoundingGeometry & bg);
};

} // namespace Collide


#endif
