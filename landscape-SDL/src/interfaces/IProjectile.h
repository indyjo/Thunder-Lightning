#ifndef IPROJECTILE_H
#define IPROJECTILE_H

#include <interfaces/IActor.h>
#include <modules/math/Vector.h>

class IProjectile: virtual public IActor
{
public:
    virtual void shoot(
        const Vector &pos, const Vector &vec, const Vector &dir)=0;
};

#endif
