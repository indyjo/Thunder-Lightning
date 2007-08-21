#ifndef TNL_WATER_H
#define TNL_WATER_H

#include <interfaces/IGame.h>
#include <object.h>

class SceneRenderPass;

class WaterImpl;
class Water : public Object {
    Ptr<WaterImpl> pImpl;
public:
    Water(Ptr<IGame>);
    ~Water();
    
    bool needsRenderPass();
    void draw(SceneRenderPass *);
    void update();
};



#endif

