#ifndef TNL_RENDERCONTEXT_H
#define TNL_RENDERCONTEXT_H

#include <interfaces/ICamera.h>
#include <object.h>

struct ICamera;

/// A RenderContext if a description of how and what to draw in the scene.
/// IDrawable implementors can query the game's current RenderContext by calling
/// IGame::getCurrentRenderContext(). Based on the description in the context,
/// They may draw themselves differently or not at all.

struct RenderContext {
    /// The semantical type of this render. Special-case render methods (like,e.g.
    /// night vision) should add an own type name.
    enum {MAIN, MIRROR} type;
    
    RenderContext(Ptr<ICamera> camera);
    ~RenderContext();
    
    Ptr<ICamera> camera;
    bool clip_above_water, clip_below_water;
    bool    draw_skybox,
            draw_terrain,
            draw_actors,
            draw_water,
            draw_gunsight,
            draw_console;
};

#endif

