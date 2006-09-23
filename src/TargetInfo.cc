#include "TargetInfo.h"
const Type TargetInfo::NONE("None",0);
const Type TargetInfo::VIRTUAL("Virtual",0);
const Type TargetInfo::STATIC("Static",0);
const Type TargetInfo::GUIDED("Guided",0);
const Type TargetInfo::DETECTABLE("Detectable",0);
const Type TargetInfo::AERIAL("Aerial",0);
const Type TargetInfo::BALLISTIC("Ballistic", &AERIAL, 0);
const Type TargetInfo::AIRCRAFT("Aircraft", &AERIAL, &GUIDED, &DETECTABLE, 0);
const Type TargetInfo::MISSILE("Missile", &AERIAL, 0);
const Type TargetInfo::VEHICLE("Vehicle", &GUIDED, &DETECTABLE, 0);
const Type TargetInfo::BUILDING("Building",0);
const Type TargetInfo::DEBUG("Debug", &VIRTUAL, 0);
const Type TargetInfo::NAVPOINT("Navpoint", &VIRTUAL, &STATIC, 0);
const Type TargetInfo::AIMINGHELPER("Aiming Helper", &VIRTUAL, 0);
const Type TargetInfo::BULLET("Bullet", &BALLISTIC, 0);
const Type TargetInfo::DUMB_MISSILE("Dumb Missile", &MISSILE, 0);
const Type TargetInfo::GUIDED_MISSILE("Guided Missile", &MISSILE, &GUIDED, 0);
const Type TargetInfo::TANK("Tank", &VEHICLE, 0);

