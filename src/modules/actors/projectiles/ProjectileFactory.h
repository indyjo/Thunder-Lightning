#ifndef PROJECTILE_FACTORY_H
#define PROJECTILE_FACTORY_H

#include <interfaces/IProjectile.h>
#include <Factory.h>

struct IGame;

typedef IFactory3<IProjectile, Ptr<IGame>, Ptr<IActor>, Ptr<IActor> > IProjectileFactory;

struct ProjectileFactories {
    IProjectileFactory * dumb_missile_factory;
    //IProjectileFactory * smart_missile_factory;
    IProjectileFactory * smart_missile_2_factory;

    ProjectileFactories();

    static ProjectileFactories factories;
};

#endif

