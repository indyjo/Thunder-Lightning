#include "RenderContext.h"

RenderContext::RenderContext(Ptr<ICamera> cam) {
    type = MAIN;
    camera = cam;
    clip_above_water = false;
    clip_below_water = false;
    draw_skybox = true;
    draw_terrain = true;
    draw_actors = true;
    draw_water = true;
    draw_gunsight = true;
    draw_console = true;
}

RenderContext::~RenderContext()
{ }

