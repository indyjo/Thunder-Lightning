#ifndef TNL_WATER_H
#define TNL_WATER_H

#include <interfaces/IGame.h>
#include <object.h>

class RenderPass;

class WaterImpl;
class Water : public Object {
    Ptr<WaterImpl> pImpl;
public:
    Water(Ptr<IGame>);
    ~Water();
    
    void draw();
    Ptr<RenderPass> createRenderPass();
    void linkRenderPassToCamera(Ptr<RenderPass> render_pass, Ptr<ICamera> cam);
};



#endif

