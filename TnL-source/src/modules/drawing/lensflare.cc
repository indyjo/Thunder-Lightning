#include "lensflare.h"
#include "billboard.h"

FlareParams::FlareParams(Ptr<TextureManager> texman, Ptr<IConfig> cfg)
: half_size(1)
{
    reflex_tex = texman->query( cfg->query( "Lensflare_reflex_tex" ) );
    sparkle_thin_tex = texman->query( cfg->query( "Lensflare_sparkle_thin_tex" ) );
    sparkle_thick_tex = texman->query( cfg->query( "Lensflare_sparkle_thick_tex" ) );
    rot_speed_thin = cfg->queryFloat( "Lensflare_rot_speed_thin", 0.0f );
    rot_speed_thick = cfg->queryFloat( "Lensflare_rot_speed_thick", 0.0f );
}

void drawLensFlare(JRenderer *renderer,
                   const Vector & pos,
                   Ptr<IPositionProvider> observer,
                   FlareParams & params,
                   float intensity,
                   float age
                  )
{
    renderer->enableAlphaBlending();
    renderer->setBlendMode(JR_BLENDMODE_ADDITIVE);
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->enableTexturing();
    
    renderer->disableZBufferWriting();
    renderer->disableFog();
    
    renderer->setColor(Vector(1,1,1));

    renderer->setAlpha(intensity);
    renderer->setTexture(params.reflex_tex->getTxtid());
    drawBillboard(renderer, pos, observer, 0, params.half_size, params.half_size);

    renderer->setAlpha(intensity*intensity);
    renderer->setTexture(params.sparkle_thin_tex->getTxtid());
    drawBillboard(renderer, pos, observer, age*params.rot_speed_thin, params.half_size, params.half_size);

    renderer->setAlpha(intensity*intensity*intensity);
    renderer->setTexture(params.sparkle_thick_tex->getTxtid());
    drawBillboard(renderer, pos, observer, age*params.rot_speed_thick, params.half_size, params.half_size);

    renderer->enableFog();
    renderer->enableZBufferWriting();
    renderer->setBlendMode(JR_BLENDMODE_BLEND);
    renderer->disableAlphaBlending();
    renderer->disableTexturing();
}
