#ifndef _JOGI_OPENGL_H
#define _JOGI_OPENGL_H

#include "JRenderer.h"

#define JGL_MAX_TEXTURES 256

typedef enum {
    JGL_FRUSTUM_LEFT=0,
    JGL_FRUSTUM_RIGHT,
    JGL_FRUSTUM_BOTTOM,
    JGL_FRUSTUM_TOP,
    JGL_FRUSTUM_NEAR,
    JGL_FRUSTUM_FAR
} jgl_frustum_t;

typedef struct {
    int used;
    unsigned int gl_tex_name;
    float size_w, size_h;
} jgl_texture_t;

class JOpenGLRenderer : public JRenderer
{
public:

    /* Initialisation methods ---------------------------------------*/
    JOpenGLRenderer(int init_width, int init_height);

    virtual void setVertexMode(jrvertexmode_t mode);
    virtual void setCoordSystem(jrcoordsystem_t cs);
    
    virtual void setCullMode(jrcullmode_t mode);
    virtual void setClipRange(float near, float far);

    virtual void setCamera(jcamera_t *cam);

    virtual void setBackgroundColor(const jcolor3_t *col);

    /* Shading methods ----------------------------------------------*/
    virtual void enableSmoothShading();
    virtual void disableSmoothShading();
    
    /* Alpha blending methods ---------------------------------------*/
    virtual void enableAlphaBlending();
    virtual void disableAlphaBlending();
    virtual jBool alphaBlendingEnabled();
    virtual void setBlendMode(jrblendmode_t);

    /* Z-Buffer methods ---------------------------------------------*/

    virtual void setZBufferFunc(jrzbfunc_t func);
    virtual void enableZBufferReading();
    virtual void disableZBufferReading();
    virtual void enableZBufferWriting();
    virtual void disableZBufferWriting();
    virtual void enableZBuffer();
    virtual void disableZBuffer();

    /* Gamma correction methods -------------------------------------*/

    virtual float getGammaCorrectionValue();
    virtual void  setGammaCorrectionValue(float val);

    /* Drawing methods ----------------------------------------------*/

    virtual void begin(jrdrawmode_t mode);
    virtual void end();

    virtual void addVertex(jvertex_col    *v);
    virtual void addVertex(jvertex_txt    *v);
    virtual void addVertex(jvertex_coltxt *v);

    virtual void setAlpha(float alpha);
    virtual void setColor(const Vector &);
    virtual void setUVW(const Vector &);
    virtual void setAbsoluteUVW(const Vector &);
    virtual void vertex(const Vector &);
    
    virtual void flush();

    virtual void clear();

    /* Texturing methods --------------------------------------------*/

    virtual void enableTexturing();
    virtual void disableTexturing();
    virtual bool texturingEnabled();

    virtual unsigned int getMaxCompression(unsigned int hint);

    virtual jError createTexture(const jsprite_t *sprite,
                                 unsigned int hint,
                                 unsigned int compression,
                                 jBool mipmap,
                                 jrtxtid_t *dst);

    virtual jError destroyTexture(jrtxtid_t txtid);

    virtual jError setTexture(jrtxtid_t txtid);
    
    virtual void setWrapMode(jrtexdim_t dim, jrwrapmode_t mode);
    
    /* Fogging methods ----------------------------------------------*/

    virtual void   setFogColor(const jcolor3_t *col);
    virtual void   getFogColor(jcolor3_t *col);
    virtual jError setFogType(jrfogtype_t type, float density);
    
    virtual jError enableFog();
    virtual jError disableFog();
    virtual jBool  fogEnabled();

    /* Matrix stack -------------------------------------------------*/
    virtual void pushMatrix();
    virtual void setMatrix(const Matrix &);
    virtual void multMatrix(const Matrix &);
    virtual void popMatrix();
    
    /* Status requesting methods ------------------------------------*/
    virtual void resize(int new_width, int new_height);

private:
    void initProjectionMatrix();
    void initModelViewMatrix();
    void uploadTexture(int level, int format, int w, int h, ju32 *src);
    void setupFog();
    void initTextureArray();
    int findFreeTexture();
    void textureScaleDown(int w, int h, ju32 *src, ju32 *dst);

private:
    float view_frustum[6];
    float fog_color[4];
    jrfogtype_t fog_type;
    float fog_density;
    float  alpha;
    Vector color;
    Vector uvw;
    jrcoordsystem_t coord_sys;
    JCamera camera;
    jgl_texture_t texture[JGL_MAX_TEXTURES];
    int current_tex;
};


#endif
