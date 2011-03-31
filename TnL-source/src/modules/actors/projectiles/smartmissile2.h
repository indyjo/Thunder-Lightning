#ifndef SMARTMISSILE2_H
#define SMARTMISSILE2_H

#include <tnl.h>

#include "Missile.h"

#include <modules/math/Differential.h>

class SmartMissile2: public Missile
{
public:
    SmartMissile2(Ptr<IGame> thegame, Ptr<IActor> target, Ptr<IActor> source=0);

    virtual void action();

private:
    void interceptTarget(float delta_t);

private:
    Ptr<IActor> target;
    float last_decoy_check;
    Integral<Vector> I_error_dt;
    Differential<Vector> d_error_dt;
};


#endif
