#ifndef PROJECTILE_LAUNCHER_H
#define PROJECTILE_LAUNCHER_H

#include <interfaces/IGame.h>
#include <modules/actors/projectiles/ProjectileFactory.h>
#include "Targeter.h"
#include "Weapon.h"

class ProjectileLauncher : public Weapon {
    Ptr<IGame> game;
    Ptr<Targeter> targeter;
    IProjectileFactory *factory;
public:
    ProjectileLauncher(
        Ptr<IGame> game,
        Ptr<Targeter> targeter,
        IProjectileFactory * factory,
        const std::string & name,
        int rounds);

    virtual bool canFire();
    virtual WeakPtr<IActor> onFire();

    float launch_speed;
    bool needs_target;
};

#endif
