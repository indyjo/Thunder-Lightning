#ifndef ARMAMENT_H
#define ARMAMENT_H

#include <vector>
#include <interfaces/IDrawable.h>
#include <landscape.h>
#include "Weapon.h"

struct IActor;
namespace Collide {
    struct Collidable;
}

class Armament : public SigC::Object
{
    std::vector<Ptr<Weapon> > weapons;
    int current;
    IActor *actor;
    Collide::Collidable *nocollide;
public:
    Armament(IActor *actor, Collide::Collidable *nocollide);
    virtual ~Armament();
    
    void addWeapon(Ptr<Weapon>);
    void nextWeapon();
    void previousWeapon();
    Ptr<Weapon> currentWeapon();
    
    const char * getWeaponName();
    void trigger();
    void release();
    
    void draw(JRenderer *);
    void action(float delta_t);
    
    inline IActor *getSourceActor() { return actor; }
    inline Collide::Collidable *getNoCollideParent() { return nocollide; }
};

#endif
