#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <tnl.h>
#include <interfaces/IPositionProvider.h>

class Environment : public Object {
    Vector fog_color;
    float clip_min, clip_max;
    float ground_fog_min, ground_fog_max, ground_fog_range;
    Vector p;
    
public:
    Environment();
    
    void update(Ptr<IPositionProvider>);

    inline float getClipMin() { return clip_min; }
    inline float getClipMax() { return clip_max; }

    inline void setClipMin(float f) { clip_min = f; }
    inline void setClipMax(float f) { clip_max = f; }
    
    inline const Vector & getFogColor() { return fog_color; }
    inline float getGroundFogMin() { return ground_fog_min; }
    inline float getGroundFogMax() { return ground_fog_max; }
    inline float getGroundFogRange() { return ground_fog_range; }
    
    inline void setFogColor(Vector c) { fog_color = c; }
    inline void setGroundFogMin(float f) { ground_fog_min = f; }
    inline void setGroundFogMax(float f) { ground_fog_max = f; }
    inline void setGroundFogRange(float f) { ground_fog_range = f; }
    
    float getGroundFogStrengthAt(const Vector &);
    float getNormalFogStrengthAt(const Vector &);
    inline float getFogStrengthAt(const Vector &v) {
        //return min(1.0f, getGroundFogStrengthAt(v) + getNormalFogStrengthAt(v));
        float f1 = getGroundFogStrengthAt(v);
        float f2 = getNormalFogStrengthAt(v);
        return 1.0 - (1.0 - f1) * (1.0 - f2);
    }
};


#endif
