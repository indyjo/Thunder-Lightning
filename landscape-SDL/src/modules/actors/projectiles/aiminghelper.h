#ifndef AIMINGHELPER_H
#define AIMINGHELPER_H

#include <modules/math/Rendezvous.h>
#include <modules/actors/simpleactor.h>
#include <landscape.h>

class AimingHelper : public SimpleActor
{
    Rendezvous rendezvous;
public:
    AimingHelper(Ptr<IGame> game)
    : SimpleActor(game)
    {
        setTargetInfo(new TargetInfo(
            "Aiming helper", 5.0f, TargetInfo::CLASS_AIMINGHELPER));
    }
    
    virtual void action() { }
    void kill() { state = DEAD; }
    void update( double delta_t,
                 const Vector & pt, const Vector & vt,
                 const Vector & ps, const Vector & vs,
                 float vel );
};


#endif
