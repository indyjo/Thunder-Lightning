#include "environment.h"

// #define FOG_COLOR_RED   240.0f
// #define FOG_COLOR_GREEN 240.0f
// #define FOG_COLOR_BLUE  240.0f

//#define FOG_COLOR_RED   235.0f
//#define FOG_COLOR_GREEN 242.0f
//#define FOG_COLOR_BLUE  251.0f

// Sunset background
//#define FOG_COLOR_RED   234.0f
//#define FOG_COLOR_GREEN 193.0f
//#define FOG_COLOR_BLUE  139.0f

// Pastel background
#define FOG_COLOR_RED   202.0f
#define FOG_COLOR_GREEN 164.0f
#define FOG_COLOR_BLUE  145.0f

// Test background
//#define FOG_COLOR_RED   0.0f
//#define FOG_COLOR_GREEN 255.0f
//#define FOG_COLOR_BLUE  0.0f

#define CLIP_MIN_RANGE     2.0f
#define CLIP_MAX_RANGE 20000.0f

#define GROUND_FOG_MIN      0.0f
#define GROUND_FOG_MAX    400.0f
#define GROUND_FOG_RANGE 3500.0f

Environment::Environment() :
    fog_color(
        Vector(FOG_COLOR_RED, FOG_COLOR_GREEN, FOG_COLOR_BLUE) / 256),
    clip_min(CLIP_MIN_RANGE),
    clip_max(CLIP_MAX_RANGE),
    ground_fog_min(GROUND_FOG_MIN),
    ground_fog_max(GROUND_FOG_MAX),
    ground_fog_range(GROUND_FOG_RANGE)
{
    ls_message("Fog color: %f %f %f\n",
            FOG_COLOR_RED,
            FOG_COLOR_GREEN,
            FOG_COLOR_BLUE);
}

void Environment::update(Ptr<IPositionProvider> pp) {
    p = pp->getLocation();
}

float Environment::getGroundFogStrengthAt(const Vector & v) {
    Vector p2(p);
    Vector v2(v);
    
    if ((p2[1] < ground_fog_min && v2[1] < ground_fog_min)
        || (p2[1] > ground_fog_max && v2[1] > ground_fog_max))
    {
        return 0;
    }
    
    if ( p2[1] > ground_fog_max ) {
        p2 += (v-p2) * ((ground_fog_max-p2[1])/(v[1] - p2[1]));
    } else if ( p2[1] < ground_fog_min ) {
        p2 += (v-p2) * ((ground_fog_min-p2[1])/(v[1] - p2[1]));
    }
    if ( v2[1] > ground_fog_max ) {
        v2 += (p2-v2) * ((ground_fog_max-v2[1])/(p2[1] - v2[1]));
    } else if ( v2[1] < ground_fog_min ) {
        v2 += (p2-v2) * ((ground_fog_min-v2[1])/(p2[1] - v2[1]));
    }

    return std::min(1.0f, (p2-v2).length() / ground_fog_range);
}

float Environment::getNormalFogStrengthAt(const Vector & v) {
    return std::min(1.0f, (v-p).length() / clip_max);
}
