#include "TargetInfo.h"

const TargetInfo::TargetClass TargetInfo::CLASS_NONE = {
    "None", // string name;
    0,      // int class_id;
    true,   // bool is_virtual:1;
    false,  // bool is_stationary:1;
    false,  // bool is_manned:1;
    false,  // bool is_controlled:1;
    false,  // bool is_aerial:1;
    false,  // bool is_nautical:1;
    false,  // bool is_subnautical:1;
    false,  // bool is_navigational:1;
    false,  // bool is_building:1;
    false,  // bool is_person:1;
    false,  // bool is_weapon:1;
    false,  // bool is_armed:1;
    false,  // bool is_damageable:1;
    false,  // bool is_radar_detectable:1;
    false,  // bool is_sonar_detectable:1;
    false   // bool is_infrared_detectable:1;
};


const TargetInfo::TargetClass TargetInfo::CLASS_DEBUG = {
    "Debug", // string name;
    1,      // int class_id;
    true,   // bool is_virtual:1;
    false,  // bool is_stationary:1;
    false,  // bool is_manned:1;
    false,  // bool is_controlled:1;
    false,  // bool is_aerial:1;
    false,  // bool is_nautical:1;
    false,  // bool is_subnautical:1;
    false,  // bool is_navigational:1;
    false,  // bool is_building:1;
    false,  // bool is_person:1;
    false,  // bool is_weapon:1;
    false,  // bool is_armed:1;
    false,  // bool is_damageable:1;
    false,  // bool is_radar_detectable:1;
    false,  // bool is_sonar_detectable:1;
    false   // bool is_infrared_detectable:1;
};


const TargetInfo::TargetClass TargetInfo::CLASS_NAVPOINT = {
    "Navpoint", // string name;
    2,      // int class_id;
    true,   // bool is_virtual:1;
    true,   // bool is_stationary:1;
    false,  // bool is_manned:1;
    false,  // bool is_controlled:1;
    false,  // bool is_aerial:1;
    false,  // bool is_nautical:1;
    false,  // bool is_subnautical:1;
    true,   // bool is_navigational:1;
    false,  // bool is_building:1;
    false,  // bool is_person:1;
    false,  // bool is_weapon:1;
    false,  // bool is_armed:1;
    false,  // bool is_damageable:1;
    false,  // bool is_radar_detectable:1;
    false,  // bool is_sonar_detectable:1;
    false   // bool is_infrared_detectable:1;
};


const TargetInfo::TargetClass TargetInfo::CLASS_BUILDING = {
    "Building", // string name;
    3,      // int class_id;
    false,  // bool is_virtual:1;
    true,   // bool is_stationary:1;
    true,   // bool is_manned:1;
    false,  // bool is_controlled:1;
    false,  // bool is_aerial:1;
    false,  // bool is_nautical:1;
    false,  // bool is_subnautical:1;
    false,  // bool is_navigational:1;
    true,   // bool is_building:1;
    false,  // bool is_person:1;
    false,  // bool is_weapon:1;
    false,  // bool is_armed:1;
    true,   // bool is_damageable:1;
    true,   // bool is_radar_detectable:1;
    false,  // bool is_sonar_detectable:1;
    false   // bool is_infrared_detectable:1;
};


const TargetInfo::TargetClass TargetInfo::CLASS_BULLET = {
    "Bullet", // string name;
    4,      // int class_id;
    false,  // bool is_virtual:1;
    false,  // bool is_stationary:1;
    false,  // bool is_manned:1;
    false,  // bool is_controlled:1;
    true,   // bool is_aerial:1;
    false,  // bool is_nautical:1;
    false,  // bool is_subnautical:1;
    false,  // bool is_navigational:1;
    false,  // bool is_building:1;
    false,  // bool is_person:1;
    true,   // bool is_weapon:1;
    false,  // bool is_armed:1;
    false,  // bool is_damageable:1;
    false,  // bool is_radar_detectable:1;
    false,  // bool is_sonar_detectable:1;
    false   // bool is_infrared_detectable:1;
};


const TargetInfo::TargetClass TargetInfo::CLASS_MISSILE = {
    "Missile", // string name;
    5,      // int class_id;
    false,  // bool is_virtual:1;
    false,  // bool is_stationary:1;
    false,  // bool is_manned:1;
    false,  // bool is_controlled:1;
    true,   // bool is_aerial:1;
    false,  // bool is_nautical:1;
    false,  // bool is_subnautical:1;
    false,  // bool is_navigational:1;
    false,  // bool is_building:1;
    false,  // bool is_person:1;
    true,   // bool is_weapon:1;
    false,  // bool is_armed:1;
    true,   // bool is_damageable:1;
    true,   // bool is_radar_detectable:1;
    false,  // bool is_sonar_detectable:1;
    true    // bool is_infrared_detectable:1;
};


const TargetInfo::TargetClass TargetInfo::CLASS_GUIDED_MISSILE = {
    "Guided missile", // string name;
    6,      // int class_id;
    false,  // bool is_virtual:1;
    false,  // bool is_stationary:1;
    false,  // bool is_manned:1;
    true,   // bool is_controlled:1;
    true,   // bool is_aerial:1;
    false,  // bool is_nautical:1;
    false,  // bool is_subnautical:1;
    false,  // bool is_navigational:1;
    false,  // bool is_building:1;
    false,  // bool is_person:1;
    true,   // bool is_weapon:1;
    false,  // bool is_armed:1;
    true,   // bool is_damageable:1;
    true,   // bool is_radar_detectable:1;
    false,  // bool is_sonar_detectable:1;
    true    // bool is_infrared_detectable:1;
};


const TargetInfo::TargetClass TargetInfo::CLASS_AIRCRAFT = {
    "Aircraft", // string name;
    7,      // int class_id;
    false,  // bool is_virtual:1;
    false,  // bool is_stationary:1;
    true,   // bool is_manned:1;
    true,   // bool is_controlled:1;
    true,   // bool is_aerial:1;
    false,  // bool is_nautical:1;
    false,  // bool is_subnautical:1;
    false,  // bool is_navigational:1;
    false,  // bool is_building:1;
    false,  // bool is_person:1;
    false,  // bool is_weapon:1;
    true,   // bool is_armed:1;
    true,   // bool is_damageable:1;
    true,   // bool is_radar_detectable:1;
    false,  // bool is_sonar_detectable:1;
    true    // bool is_infrared_detectable:1;
};


const TargetInfo::TargetClass TargetInfo::CLASS_TANK = {
    "Tank", // string name;
    8,      // int class_id;
    false,  // bool is_virtual:1;
    false,  // bool is_stationary:1;
    true,   // bool is_manned:1;
    true,   // bool is_controlled:1;
    false,  // bool is_aerial:1;
    false,  // bool is_nautical:1;
    false,  // bool is_subnautical:1;
    false,  // bool is_navigational:1;
    false,  // bool is_building:1;
    false,  // bool is_person:1;
    false,  // bool is_weapon:1;
    true,   // bool is_armed:1;
    true,   // bool is_damageable:1;
    true,   // bool is_radar_detectable:1;
    false,  // bool is_sonar_detectable:1;
    true    // bool is_infrared_detectable:1;
};


const TargetInfo::TargetClass TargetInfo::CLASS_AIMINGHELPER = {
    "Aiming helper", // string name;
    9,      // int class_id;
    true,   // bool is_virtual:1;
    false,  // bool is_stationary:1;
    false,  // bool is_manned:1;
    false,  // bool is_controlled:1;
    false,  // bool is_aerial:1;
    false,  // bool is_nautical:1;
    false,  // bool is_subnautical:1;
    false,  // bool is_navigational:1;
    false,  // bool is_building:1;
    false,  // bool is_person:1;
    false,  // bool is_weapon:1;
    false,  // bool is_armed:1;
    false,  // bool is_damageable:1;
    false,  // bool is_radar_detectable:1;
    false,  // bool is_sonar_detectable:1;
    false,  // bool is_infrared_detectable:1;
};
