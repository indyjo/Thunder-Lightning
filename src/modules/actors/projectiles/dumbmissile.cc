#include <interfaces/IConfig.h>
#include <modules/engines/effectors.h>
#include <modules/engines/rigidengine.h>
#include <modules/model/modelman.h>
#include "dumbmissile.h"

// A dumb missile inspired by the Hydra 70 with data from
// http://www.fas.org/man/dod-101/sys/missile/hydra-70.htm

DumbMissile::DumbMissile(Ptr<IGame> thegame, Ptr<IActor>, Ptr<IActor> source)
    : Missile(thegame, source, "DumbMissile")
{
    // nothing to do anymore?
}


