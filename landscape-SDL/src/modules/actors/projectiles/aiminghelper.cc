#include "aiminghelper.h"
#include <modules/math/Solve.h>

void AimingHelper::update( double delta_t,
                           const Vector & pt, const Vector & vt,
                           const Vector & ps, const Vector & vs,
                           float vel ) {
    rendezvous.updateSource(ps, vs, Vector(0,-9.81f,0));
    rendezvous.updateTarget(delta_t, pt, vt);
    rendezvous.setVelocity(vel);
    Vector new_pos = rendezvous.calculate();
    setLocation( new_pos );
    //ls_warning("new Target position: ");
    //new_pos.dump();
}
