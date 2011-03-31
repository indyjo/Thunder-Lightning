#ifndef TNL_TEXTURE_H
#define TNL_TEXTURE_H

#include <Weak.h>
#include <modules/jogi/JRenderer.h>

class Texture : public Object, public Weak
{
    jrtxtid_t tex;
    JRenderer &renderer;
    
protected:
    ~Texture();
    
public:
    Texture(jrtxtid_t t, JRenderer &r);

    inline int getWidth() { return renderer.getTextureWidth(tex); }
    inline int getHeight() { return renderer.getTextureHeight(tex); }
    
    inline jrtxtid_t getTxtid() { return tex; }
    inline unsigned int getGLTex() { return renderer.getGLTexFromTxtid(tex); }
};

typedef Ptr<Texture> TexPtr;


#endif

