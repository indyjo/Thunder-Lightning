#include "Armament.h"

Armament::Armament(IActor *actor, Collide::Collidable *nocollide)
: current(0), actor(actor), nocollide(nocollide) { }
Armament::~Armament() { }

void Armament::addWeapon(Ptr<Weapon> w) {
    weapons.push_back(w);
}

void Armament::nextWeapon() {
    if (weapons.size()==0) return;
    currentWeapon()->release();
    if (++current == weapons.size())
        current = 0;
}

void Armament::previousWeapon(){
    if (weapons.size()==0) return;
    currentWeapon()->release();
    if (current-- == 0)
        current = weapons.size()-1;
}

Ptr<Weapon> Armament::currentWeapon() {
    if (weapons.size()==0) return 0;
    return weapons[current];
}

const char * Armament::getWeaponName() { return currentWeapon()?currentWeapon()->getName():"N/A"; }
void Armament::trigger() { if (!currentWeapon()) return; currentWeapon()->trigger(*this); }
void Armament::release() { if (!currentWeapon()) return; currentWeapon()->release(); }

void Armament::draw(JRenderer *r) {
    for (int i=0; i<weapons.size(); ++i)
        weapons[i]->draw(r);
}

void Armament::action(float delta_t) {
    for (int i=0; i<weapons.size(); ++i)
        weapons[i]->action(delta_t, *this);
}

