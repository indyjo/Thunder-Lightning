#include "TargetInfo.h"
const Type TargetInfo::VIRTUAL("Virtual",0);
const Type TargetInfo::PHYSICAL("Physical",0);
const Type TargetInfo::NONE("None", &VIRTUAL, 0);
const Type TargetInfo::STATIC("Static",0);
const Type TargetInfo::GUIDED("Guided",0);
const Type TargetInfo::DETECTABLE("Detectable",0);
const Type TargetInfo::DECOY("Decoy",0);
const Type TargetInfo::AERIAL("Aerial",0);
const Type TargetInfo::BALLISTIC("Ballistic", &AERIAL, &PHYSICAL, 0);
const Type TargetInfo::AIRCRAFT("Aircraft", &AERIAL, &GUIDED, &DETECTABLE, &PHYSICAL, 0);
const Type TargetInfo::MISSILE("Missile", &AERIAL, &PHYSICAL, 0);
const Type TargetInfo::VEHICLE("Vehicle", &GUIDED, &DETECTABLE, &PHYSICAL, 0);
const Type TargetInfo::BUILDING("Building", &STATIC, &PHYSICAL, 0);
const Type TargetInfo::NAVAL("Naval",0);
const Type TargetInfo::SHIP("Ship", &NAVAL, &DETECTABLE, &PHYSICAL,0);
const Type TargetInfo::DEBUG("Debug", &VIRTUAL, 0);
const Type TargetInfo::NAVPOINT("Navpoint", &VIRTUAL, &STATIC, 0);
const Type TargetInfo::AIMINGHELPER("Aiming Helper", &VIRTUAL, 0);
const Type TargetInfo::BULLET("Bullet", &BALLISTIC,  &PHYSICAL, 0);
const Type TargetInfo::DUMB_MISSILE("Dumb Missile", &MISSILE, 0);
const Type TargetInfo::GUIDED_MISSILE("Guided Missile", &MISSILE, &GUIDED, &DETECTABLE, 0);
const Type TargetInfo::TANK("Tank", &VEHICLE, 0);
const Type TargetInfo::CARRIER("Carrier", &SHIP, 0);
const Type TargetInfo::DECOY_FLARE("Decoy flare", &DECOY, &BALLISTIC, 0);

