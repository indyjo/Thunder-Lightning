#ifndef ARMAMENT_H
#define ARMAMENT_H

#include <vector>
#include <interfaces/IDrawable.h>
#include <tnl.h>
#include "Weapon.h"

struct IActor;
namespace Collide {
    class Collidable;
}

class Armament : public SigObject
{
    typedef std::vector<Ptr<Weapon> > Weapons;
    struct WeaponGroup {
        Weapons weapons;
        int current;
    };
    typedef std::vector<WeaponGroup> WeaponGroups;
    
    WeaponGroups weapon_groups;
    IActor *actor;
    Collide::Collidable *nocollide;
public:
    typedef size_t groupid;
    
    Armament(IActor *actor, Collide::Collidable *nocollide);
    virtual ~Armament();
    
    /// Add a new weapon to the specified weapon group
    void addWeapon(groupid, Ptr<Weapon>);
    
    /// Select next weapon in group
    void nextWeapon(groupid);
    /// Select previous weapon in group
    void previousWeapon(groupid);
    /// Get currently selected weapon of group
    Ptr<Weapon> currentWeapon(groupid);
    
    /// Returns a vector of pointers to all the weapons in the armament.
    Weapons getAllWeapons();

    /// Gets weapon with given name from any group
    Ptr<Weapon> getWeapon(const std::string & name);
    
    void trigger(groupid);
    void release(groupid);
    
    void draw(JRenderer *);
    void action(float delta_t);
    
    inline IActor *getSourceActor() { return actor; }
    inline Collide::Collidable *getNoCollideParent() { return nocollide; }
};

#endif
