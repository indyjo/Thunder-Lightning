#ifndef TNL_SCENERENDERPASS_H
#define TNL_SCENERENDERPASS_H

#include <interfaces/IGame.h>
#include <modules/camera/MirrorCamera.h>
#include <modules/texman/TextureManager.h>
#include "RenderPass.h"
#include "RenderContext.h"
#include "Weak.h"

/// A render pass that draws the game's main 3d scene, including actors,
/// according to a description given in a RenderContext.
class SceneRenderPass : public RenderPass
{
public:
    SceneRenderPass(Ptr<IGame>, const RenderContext &);
    ~SceneRenderPass();

    inline Ptr<Texture> getMirrorTexture() { return mirror_tex; }
    inline Ptr<MirrorCamera> getMirrorCamera() { return mirror_cam; }

    /// Public render context to configure the scene rendering
    RenderContext context;
protected:
    void draw();

    WeakPtr<IGame> thegame;
    Ptr<Texture> mirror_tex;
    Ptr<MirrorCamera> mirror_cam;
};

#endif

