#ifndef TARGETINFO_H
#define TARGETINFO_H

#include <landscape.h>
#include <object.h>
#include <string>

class TargetInfo: virtual public Object {
public:
    struct TargetClass {
        std::string name;
        int class_id;
        bool is_virtual:1;
        bool is_stationary:1;
        bool is_manned:1;
        bool is_controlled:1;
        bool is_aerial:1;
        bool is_nautical:1;
        bool is_subnautical:1;
        bool is_navigational:1;
        bool is_building:1;
        bool is_person:1;
        bool is_weapon:1;
        bool is_armed:1;
        bool is_damageable:1;
        bool is_radar_detectable:1;
        bool is_sonar_detectable:1;
        bool is_infrared_detectable:1;
    };

    static const TargetClass    CLASS_NONE,
                                CLASS_DEBUG,
                                CLASS_NAVPOINT,
                                CLASS_BUILDING,
                                CLASS_BULLET,
                                CLASS_MISSILE,
                                CLASS_GUIDED_MISSILE,
                                CLASS_AIRCRAFT,
                                CLASS_TANK,
                                CLASS_AIMINGHELPER;

public:
    TargetInfo( const std::string & name,
                float size,
                TargetClass tclass )
    :   ti_size(size), ti_name(name), ti_tclass(tclass)
    { }


    std::string getTargetName() { return ti_name; }
    void setTargetName(const std::string & name) { ti_name = name; }

    std::string getTargetInfo() { return ti_info; }
    void setTargetInfo(const std::string & info) { ti_info = info; }

    float getTargetSize() { return ti_size; }
    void setTargetSize(float size) { ti_size = size; }

    TargetClass getTargetClass() { return ti_tclass; }
    void setTargetClass(const TargetClass & tclass) { ti_tclass = tclass; }

private:
    float ti_size;
    std::string ti_name, ti_info;
    TargetClass ti_tclass;
};

#endif
