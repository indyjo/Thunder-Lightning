#ifndef TNL_WATER_H
#define TNL_WATER_H

#include <interfaces/IGame.h>
#include <object.h>

class WaterImpl;
class Water : public Object {
    Ptr<WaterImpl> pImpl;
public:
    Water(Ptr<IGame>);
    ~Water();
    
    void draw();
    void copyTex();
    bool supportsMirrorTex();
};



#endif

