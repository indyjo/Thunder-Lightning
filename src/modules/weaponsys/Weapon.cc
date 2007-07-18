#include <interfaces/IPositionProvider.h>

#include "Weapon.h"

Weapon::Weapon(Ptr<IConfig> cfg, const std::string & name, int rounds)
:   name(name), rounds(rounds), maxrounds(rounds),
    next_barrel(0), triggered(false), armament(0)
{
    loadtime = cfg->queryFloat(name+"_loadtime", 0.5f);
    singleshot = cfg->queryBool(name+"_singleshot", true);
    is_guided = cfg->queryBool(name+"_guided", false);
    max_range = cfg->queryFloat(name+"_max_range", 1000.0f);
    reference_speed = cfg->queryFloat(name+"_reference_speed", 300.0f);
    time_of_accel = cfg->queryFloat(name+"_time_of_accel", 0.0f);
}

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
        last_fired_round = onFire();
        fire_signal.emit(this);
        --rounds;
        ++next_barrel;
        if (next_barrel == barrels.size()) next_barrel=0;
    }
}

bool  Weapon::isGuided()            { return is_guided; }
float Weapon::maxRange()            { return max_range; }
float Weapon::referenceSpeed()      { return reference_speed; }
float Weapon::timeOfAcceleration()  { return time_of_accel; }

WeakPtr<IActor> Weapon::lastFiredRound() { return last_fired_round; }
SigC::Signal1<void, Ptr<IWeapon> > Weapon::onFireSig() { return fire_signal; }


