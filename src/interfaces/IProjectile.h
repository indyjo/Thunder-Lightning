#ifndef IPROJECTILE_H
#define IPROJECTILE_H

#include <interfaces/IActor.h>
#include <Weak.h>

namespace Collide {
    class Collidable;
}

struct IProjectile: virtual public IActor, virtual public Weak
{
    virtual void shoot(const Vector &pos, const Vector &vec, const Vector &dir)=0;
    virtual Ptr<IActor> getSource()=0;
    
    /// Returns a pointer to the projectile casted to Collidable. May return 0
    /// if projectile doesn't inherit Collidable.
    virtual Ptr<Collide::Collidable> asCollidable()=0;
};

#endif
