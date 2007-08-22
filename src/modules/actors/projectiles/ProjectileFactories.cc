#include "dumbmissile.h"
#include "smartmissile.h"
#include "smartmissile2.h"
#include "Decoy.h"

#include "ProjectileFactory.h"

namespace {
    Factory3<IProjectile, DumbMissile, Ptr<IGame>, Ptr<IActor>, Ptr<IActor> > the_dumb_missile_factory;
    //Factory3<IProjectile, SmartMissile, Ptr<IGame>, Ptr<IActor>, Ptr<IActor> > the_smart_missile_factory;
    Factory3<IProjectile, SmartMissile2, Ptr<IGame>, Ptr<IActor>, Ptr<IActor> > the_smart_missile_2_factory;
    Factory3<IProjectile, Decoy, Ptr<IGame>, Ptr<IActor>, Ptr<IActor> > the_decoy_factory;
}

ProjectileFactories::ProjectileFactories()
:dumb_missile_factory(&the_dumb_missile_factory)
//,smart_missile_factory(&the_smart_missile_factory)
,smart_missile_2_factory(&the_smart_missile_2_factory)
,decoy_factory(&the_decoy_factory)
{ }

ProjectileFactories ProjectileFactories::factories;
