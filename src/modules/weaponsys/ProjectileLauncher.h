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
    inline ProjectileLauncher(
        Ptr<IGame> game,
        Ptr<Targeter> targeter,
        IProjectileFactory * factory,
        const std::string & name,
        int rounds, float loadtime, bool singleshot=true,
        bool needs_target=false)
        
        : Weapon(name,rounds,loadtime,singleshot)
        , game(game)
        , targeter(targeter)
        , factory(factory)
        , launch_speed(0.0f)
        , needs_target(needs_target)
    { }

    virtual bool canFire();
    virtual void onFire();

    float launch_speed;
    bool needs_target;
};

#endif
