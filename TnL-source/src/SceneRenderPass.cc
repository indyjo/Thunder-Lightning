#include <interfaces/IConfig.h>
#include <modules/environment/Water.h>
#include <debug.h>
#include "SceneRenderPass.h"


SceneRenderPass::SceneRenderPass(Ptr<IGame> game, const RenderContext& ctx)
    : RenderPass(game->getRenderer())
    , thegame(game)
    , context(ctx)
{
    enableClearColor(false);
    
    Ptr<IConfig> cfg = game->getConfig();
    if (ctx.type == RenderContext::MAIN && game->getWater()->needsRenderPass()) {
        RenderContext mirror_ctx = context;
        mirror_ctx.type = RenderContext::MIRROR;
        mirror_ctx.draw_water    = false;
        mirror_ctx.draw_gunsight = false;
        mirror_ctx.draw_console  = false;
        mirror_ctx.clip_below_water = true;

        mirror_cam = new MirrorCamera(context.camera);
        mirror_ctx.camera = mirror_cam;

        mirror_tex = this->dependsOn( new SceneRenderPass(game, mirror_ctx),
            cfg->queryInt("Water_mirror_texture_size",256),
            cfg->queryInt("Water_mirror_texture_size",256));
    }
}

SceneRenderPass::~SceneRenderPass()
{ }

void SceneRenderPass::draw()
{
    Ptr<IGame> game = thegame.lock();
    if (!game) {
        return;
    }
    game->renderScene(this);
}
