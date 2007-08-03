#include "Texture.h"

Texture::Texture(jrtxtid_t t, JRenderer &r)
: tex(t), renderer(r)
{
}

Texture::~Texture()
{
    renderer.destroyTexture(tex);
}


