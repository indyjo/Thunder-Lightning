#ifndef FACTION_H
#define FACTION_H

#include <map>
#include <string>
#include <landscape.h>
#include "object.h"

class Faction : public Object {
public:
    static const struct BasicFactions {
        BasicFactions();
        Ptr<Faction> none, privateer, rogue;
        Ptr<Faction> aggressive_faction, neutral_faction;
        Ptr<Faction> faction_a, faction_b, faction_c;
    } basic_factions;
    enum Attitude { FRIENDLY, NEUTRAL, HOSTILE };

    inline Faction(const char * name)
    : default_attitude(NEUTRAL), name(name) { }
    ~Faction();

    const std::string & getName() { return name; }

    void setDefaultAttitude(Attitude a) { default_attitude = a; }
    Attitude getDefaultAttitude() { return default_attitude; }

    void setAttitudeTowards(Ptr<Faction>, Attitude);
    Attitude getAttitudeTowards(Ptr<Faction>);

private:
    std::string name;
    Attitude default_attitude;
    std::map<Ptr<Faction>, Attitude> attitudes;
};




#endif
