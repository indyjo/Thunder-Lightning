#ifndef IPROJECTILE_H
#define IPROJECTILE_H

#include <interfaces/IActor.h>
#include <modules/math/Vector.h>

class IProjectile: virtual public IActor
{
public:
    virtual Ptr<IActor> getSource()=0;
};

#endif
