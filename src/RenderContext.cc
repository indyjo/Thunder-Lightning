#include <interfaces/ICamera.h>
#include <modules/math/MatrixVector.h>
#include <modules/camera/SimpleCamera.h>
#include "RenderContext.h"

RenderContext RenderContext::MainRenderContext(Ptr<ICamera> cam) {
    RenderContext c;
    c.type = MAIN;
    c.camera = cam;
    c.clip_above_water = false;
    c.clip_below_water = false;
    c.draw_skybox = true;
    c.draw_terrain = true;
    c.draw_actors = true;
    c.draw_water = true;
    c.draw_gunsight = true;
    c.draw_console = true;
    
    return c;
}

RenderContext RenderContext::MirroredRenderContext(Ptr<ICamera> cam) {
    RenderContext c;
    c.type = MIRROR;
    
    Vector pos = cam->getLocation();
    pos[1] = -pos[1];
    
    Vector up,right,front;
    cam->getOrientation(&up,&right,&front);
    
    right[1] = -right[1];
    front[1] = -front[1];
    up = front%right;
    
    c.camera = new SimpleCamera(cam);
    c.camera->setLocation(pos);
    c.camera->setOrientation(up, right, front);

    c.clip_above_water = false;
    c.clip_below_water = true;
    c.draw_skybox = true;
    c.draw_terrain = true;
    c.draw_actors = true;
    c.draw_water = false;
    c.draw_gunsight = false;
    c.draw_console = false;

    return c;
}

