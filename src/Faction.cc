#include "Faction.h"

Faction::BasicFactions::BasicFactions() {
    none = new Faction("None");
    privateer = new Faction("Privateer");

    rogue = new Faction("Rogue");
    rogue->setDefaultAttitude(HOSTILE);

    aggressive_faction = new Faction("Aggressive");
    aggressive_faction->setDefaultAttitude(HOSTILE);
    aggressive_faction->setAttitudeTowards(aggressive_faction, FRIENDLY);

    neutral_faction = new Faction("Neutral");
    neutral_faction->setAttitudeTowards(neutral_faction, FRIENDLY);

    faction_a = new Faction("A");
    faction_b = new Faction("B");
    faction_c = new Faction("C");
    faction_a->setAttitudeTowards(faction_a, FRIENDLY);
    faction_a->setAttitudeTowards(faction_b, HOSTILE);
    faction_a->setAttitudeTowards(faction_c, HOSTILE);

    faction_b->setAttitudeTowards(faction_a, HOSTILE);
    faction_b->setAttitudeTowards(faction_b, FRIENDLY);
    faction_b->setAttitudeTowards(faction_c, HOSTILE);

    faction_c->setAttitudeTowards(faction_a, HOSTILE);
    faction_c->setAttitudeTowards(faction_b, HOSTILE);
    faction_c->setAttitudeTowards(faction_c, FRIENDLY);
};

const Faction::BasicFactions Faction::basic_factions;

Faction::~Faction() {
}

void Faction::setAttitudeTowards(Ptr<Faction> f, Attitude a) {
    attitudes.insert(std::make_pair(f,a));
}

Faction::Attitude Faction::getAttitudeTowards(Ptr<Faction> f) {
	/*{
		std::map<Ptr<Faction>, Attitude>::iterator i;
		ls_message("Attitude of %s towards %p \"%s\":",
			getName().c_str(), &*f, f->getName().c_str());
		for(i=attitudes.begin(); i!=attitudes.end(); ++i) {
			ls_message(" %p:%s",&*i->first, 
				(i->second==FRIENDLY)?"FRIENDLY":
				((i->second==HOSTILE)?"HOSTILE":"NEUTRAL"));
		}
		ls_message("\n");
	}*/
    std::map<Ptr<Faction>, Attitude>::iterator i = attitudes.find(f);
    
    if (i==attitudes.end())
        return default_attitude;
    else
        return i->second;
}

