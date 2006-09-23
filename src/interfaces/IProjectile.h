#ifndef IPROJECTILE_H
#define IPROJECTILE_H

//#include <modules/math/Vector.h>
#include <interfaces/IActor.h>

struct IProjectile: virtual public IActor
{
    virtual void shoot(const Vector &pos, const Vector &vec, const Vector &dir)=0;
    virtual Ptr<IActor> getSource()=0;
};

#endif
