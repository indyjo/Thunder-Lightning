#ifndef FACTION_H
#define FACTION_H

#include <map>
#include <string>
#include <tnl.h>
#include "Weak.h"

class Faction : public Object, public Weak {
public:
    static const struct BasicFactions {
        BasicFactions();
        Ptr<Faction> none, privateer, rogue;
        Ptr<Faction> aggressive_faction, neutral_faction;
        Ptr<Faction> faction_a, faction_b, faction_c;
    } basic_factions;
    enum Attitude { FRIENDLY, NEUTRAL, HOSTILE };

    inline Faction(const char * name="<unnamed faction>")
    : default_attitude(NEUTRAL), name(name), color(1,1,1) { }
    ~Faction();

    inline void setName(const std::string & name) { this->name = name; }
    inline const std::string & getName() { return name; }

    void setDefaultAttitude(Attitude a) { default_attitude = a; }
    Attitude getDefaultAttitude() { return default_attitude; }

    void setAttitudeTowards(WeakPtr<Faction>, Attitude);
    Attitude getAttitudeTowards(WeakPtr<Faction>);
    
    void setColor(const Vector &);
    const Vector& getColor();

private:
    std::string name;
    Attitude default_attitude;
    std::map<WeakPtr<Faction>, Attitude> attitudes;
    Vector color;
};




#endif
