#ifndef COLLIDE_POSSIBLECONTACT_H
#define COLLIDE_POSSIBLECONTACT_H

#include <queue>

#include "ContactPartner.h"

namespace Collide {
	
union Hints;

struct PossibleContact {
    ContactPartner partners[2];
    float t0, t1;

    inline friend bool operator < (const PossibleContact & a,
                                   const PossibleContact & b) {
        // a has lower priority than b if:
        return b.t0 < a.t0 || b.t0 == a.t0 && b.t1 < a.t1;
    }

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
