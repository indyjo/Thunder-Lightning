#ifndef TARGETINFO_H
#define TARGETINFO_H

#include <tnl.h>
#include <object.h>
#include <TypedObject.h>
#include <string>

class TargetInfo: public TypedObject, virtual public Object {
public:
    static const Type           NONE,
    							
    							VIRTUAL,
    							PHYSICAL,
                                
                                STATIC,
                                
                                GUIDED,
                                
                                DETECTABLE,
                                
                                DECOY,
                                
                                AERIAL,
                                BALLISTIC,
                                AIRCRAFT,
                                MISSILE,
                                VEHICLE,
                                BUILDING,
                                NAVAL,
                                SHIP,
                                
                                DEBUG,
                                NAVPOINT,
                                AIMINGHELPER,
                                BULLET,
                                DUMB_MISSILE,
                                GUIDED_MISSILE,
                                TANK,
                                CARRIER,
                                DECOY_FLARE;

public:
    TargetInfo( const std::string & name,
                float size,
                const Type & type )
    :   TypedObject(type), ti_size(size), ti_name(name)
    { }


    std::string getTargetName() { return ti_name; }
    void setTargetName(const std::string & name) { ti_name = name; }

    std::string getTargetInfo() { return ti_info; }
    void setTargetInfo(const std::string & info) { ti_info = info; }

    float getTargetSize() { return ti_size; }
    void setTargetSize(float size) { ti_size = size; }

private:
    float ti_size;
    std::string ti_name, ti_info;
};

#endif
