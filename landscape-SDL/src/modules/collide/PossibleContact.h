#ifndef COLLIDE_POSSIBLECONTACT_H
#define COLLIDE_POSSIBLECONTACT_H

#include <vector>
#include <queue>

#include <modules/math/Interval.h>
#include "ContactPartner.h"

namespace Collide {
	
union Hints;

/// Represents an interpolated movement between two
/// hierarchical transformations
struct TransformInterval {
	std::vector<Transform> time_0, time_1;
	XTransform<Interval> interval;
	
	void subdivide(TransformInterval & ti0, TransformInterval & ti1);
};

struct PossibleContact {
    float t0, t1;
    
    ContactPartner partners[2];
    TransformInterval ti[2];
    
    inline friend bool operator < (const PossibleContact & a,
                                   const PossibleContact & b) {
        // a has lower priority than b if:
        return b.t0 < a.t0 || b.t0 == a.t0 && b.t1 < a.t1;
    }

    /// Sets partners[i] and ti[i] from the given geometry instance.
    void setPartner(int i, GeometryInstance * instance);
    
    inline bool canSubdivide() {
        return partners[0].canSubdivide() || partners[1].canSubdivide();
    }
    bool mustSubdivide();
    void subdivide(std::priority_queue<PossibleContact> & q);
    bool shouldDivideTime(const Hints & hints);
    void divideTime(std::priority_queue<PossibleContact> & q);
    bool collide(float delta_t, Hints & hints);
    bool makeContact(Contact & c, float delta_t, const Hints & hints);
};

} // namespace Collide

#endif
