#ifndef TNL_LENSFLARE_H
#define TNL_LENSFLARE_H

#include <interfaces/IConfig.h>
#include <interfaces/IPositionProvider.h>
#include <modules/texman/TextureManager.h>

struct FlareParams
{
    /// Load default flare textures.
    /// half_size will be initialized to 1.
    FlareParams(Ptr<TextureManager>, Ptr<IConfig>);
    
    Ptr<Texture> reflex_tex, sparkle_thin_tex, sparkle_thick_tex;
    float half_size;
    
    float rot_speed_thin;
    float rot_speed_thick;
};

void drawLensFlare(JRenderer *renderer,
                   const Vector & pos,
                   Ptr<IPositionProvider> observer,
                   FlareParams & params,
                   float intensity=1,
                   float age=0
                  );

#endif

