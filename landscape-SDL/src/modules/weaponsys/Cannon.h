#ifndef WEAPON_CANNON_H
#define WEAPON_CANNON_H

#include <interfaces/IGame.h>
#include "Weapon.h"

struct Collidable;

class Cannon : public Weapon {
    Ptr<IGame> game;
public:
    Cannon(Ptr<IGame> game,
        const std::string & name="Cannon",
        int rounds=40,
        float loadtime=3,
        bool singleshot=true);
    
    virtual void onFire(Armament &);
};

#endif
