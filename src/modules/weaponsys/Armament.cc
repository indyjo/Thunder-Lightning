#include <stdexcept>
#include "Armament.h"

Armament::Armament(IActor *actor, Collide::Collidable *nocollide)
: actor(actor), nocollide(nocollide) {
    // By default initialize 3 weapon groups
    for(int i=0; i<3; ++i) {
        WeaponGroup group;
        group.current = 0;
        weapon_groups.push_back(group);
    }
}
Armament::~Armament() { }

void Armament::addWeapon(groupid id, Ptr<Weapon> w) {
    while (id >= weapon_groups.size()) {
        WeaponGroup group;
        group.current = 0;
        weapon_groups.push_back(group);
    }
    WeaponGroup& group = weapon_groups[id];
    group.weapons.push_back(w);
    w->attachToArmament(*this);
}

void Armament::nextWeapon(groupid id) {
    WeaponGroup& group = weapon_groups.at(id);
    if (group.weapons.size()==0) return;
    currentWeapon(id)->release();
    if (++group.current == group.weapons.size())
        group.current = 0;
}

void Armament::previousWeapon(groupid id){
    WeaponGroup& group = weapon_groups.at(id);
    if (group.weapons.size()==0) return;
    currentWeapon(id)->release();
    if (group.current-- == 0)
        group.current = group.weapons.size()-1;
}

Ptr<Weapon> Armament::getWeapon(const std::string & name) {
    typedef WeaponGroups::iterator Iter;
    for(Iter group = weapon_groups.begin(); group != weapon_groups.end(); group++) {
        for(int i = 0; i < group->weapons.size(); ++i) {
            if (name == group->weapons[i]->getName()) {
                return group->weapons[i];
            }
        }
    }
    return 0;
}

Ptr<Weapon> Armament::currentWeapon(groupid id) {
    WeaponGroup& group = weapon_groups.at(id);
    if (group.weapons.size()==0) return 0;
    return group.weapons[group.current];
}

Armament::Weapons Armament::getAllWeapons() {
    Weapons all_weapons;
    typedef WeaponGroups::iterator Iter;
    for(Iter group = weapon_groups.begin(); group != weapon_groups.end(); group++) {
        all_weapons.insert(all_weapons.end(), group->weapons.begin(), group->weapons.end());
    }
    return all_weapons;
}

void Armament::trigger(groupid id) {
    if (!currentWeapon(id)) return; currentWeapon(id)->trigger();
}
void Armament::release(groupid id) {
    if (!currentWeapon(id)) return; currentWeapon(id)->release();
}

void Armament::draw(JRenderer *r) {
    typedef WeaponGroups::iterator Iter;
    for(Iter group = weapon_groups.begin(); group != weapon_groups.end(); group++) {
        for (int i=0; i<group->weapons.size(); ++i)
            group->weapons[i]->draw(r);
    }
}

void Armament::action(float delta_t) {
    typedef WeaponGroups::iterator Iter;
    for(Iter group = weapon_groups.begin(); group != weapon_groups.end(); group++) {
        for (int i=0; i<group->weapons.size(); ++i)
            group->weapons[i]->action(delta_t);
    }
}

