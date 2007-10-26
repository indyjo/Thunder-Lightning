#ifndef FACTION_H
#define FACTION_H

#include <map>
#include <string>
#include <tnl.h>
#include "Weak.h"

class RadarNet;

class Faction : public Object, public Weak {
public:
    static const struct BasicFactions {
        BasicFactions();
        Ptr<Faction> none, privateer, rogue;
        Ptr<Faction> aggressive_faction, neutral_faction;
        Ptr<Faction> faction_a, faction_b, faction_c;
    } basic_factions;
    enum Attitude { FRIENDLY, NEUTRAL, HOSTILE };

    Faction(const char * name="<unnamed faction>");
    ~Faction();

    inline void setName(const std::string & name) { this->name = name; }
    inline const std::string & getName() { return name; }

    void setDefaultAttitude(Attitude a) { default_attitude = a; }
    Attitude getDefaultAttitude() { return default_attitude; }

    void setAttitudeTowards(WeakPtr<Faction>, Attitude);
    Attitude getAttitudeTowards(WeakPtr<Faction>);
    
    void setColor(const Vector &);
    const Vector& getColor();
    
    Ptr<RadarNet> getRadarNet();
    
    Faction & operator=(const Faction &);

private:
    std::string name;
    Attitude default_attitude;
    std::map<WeakPtr<Faction>, Attitude> attitudes;
    Vector color;
    Ptr<RadarNet> radarnet;
};




#endif
