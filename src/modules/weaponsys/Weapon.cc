#include <interfaces/IPositionProvider.h>

#include "Weapon.h"

Weapon::Weapon(const std::string & name, int rounds, float loadtime, bool singleshot)
:   name(name), rounds(rounds), maxrounds(rounds), loadtime(loadtime),
    singleshot(singleshot), next_barrel(0), triggered(false), armament(0)
{ }

void Weapon::addBarrel(Ptr<IPositionProvider> position) {
    Barrel b = { position, loadtime+1 };
    barrels.push_back(b);
}

const char * Weapon::getName() {
    return name.c_str();
}

int Weapon::getMaxRounds() {
    return maxrounds;
}

int Weapon::getRoundsLeft() {
    return rounds;
}

void Weapon::setMaxRounds(int n) {
    maxrounds=n;
}

void Weapon::setRoundsLeft(int n) {
    rounds=n;
}

bool Weapon::canFire() {
    if (next_barrel > barrels.size())
        return false;
    return rounds > 0 && barrels[next_barrel].secs_since_fire > loadtime;
}

void Weapon::trigger() {
    if (triggered) return;
    triggered = true;
    if (singleshot && canFire()) {
        barrels[next_barrel].secs_since_fire = 0;
        onFire();
        --rounds;
        ++next_barrel;
        if (next_barrel == barrels.size()) next_barrel=0;
        triggered = false;
    } else {
        // ensure that not all barrels fire at one
        for(int i=1; i<barrels.size(); ++i) {
            int j = (i+next_barrel) % barrels.size();
            barrels[j].secs_since_fire =
                std::min(barrels[j].secs_since_fire, loadtime - i*loadtime/barrels.size());
        }
    }
}

void Weapon::release() {
    triggered = false;
}

bool Weapon::isTriggered() {
    return triggered;
}

void Weapon::draw(JRenderer *) {
}

void Weapon::action(float delta_t) {
    for (int i=0; i<barrels.size(); ++i)
        barrels[i].secs_since_fire += delta_t;
    if (next_barrel > barrels.size())
        return;
    if (!triggered)
        return;

    while(canFire()) {
        barrels[next_barrel].secs_since_fire = 0;
        onFire();
        --rounds;
        ++next_barrel;
        if (next_barrel == barrels.size()) next_barrel=0;
    }
}



