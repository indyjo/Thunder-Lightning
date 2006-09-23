#ifndef _JOGI_RENDERER_H
#define _JOGI_RENDERER_H

#include "JCamera.h"
#include "JMaterial.h"
#include "JLight.h"
#include "types.h"

typedef enum {
    JR_DRAWMODE_POINTS=0,
    JR_DRAWMODE_LINES,
    JR_DRAWMODE_CONNECTED_LINES,
    JR_DRAWMODE_TRIANGLES,
    JR_DRAWMODE_TRIANGLE_STRIP,
    JR_DRAWMODE_TRIANGLE_FAN
} jrdrawmode_t;

typedef enum {
    JR_VERTEXMODE_GOURAUD=0,
    JR_VERTEXMODE_TEXTURE,
    JR_VERTEXMODE_GOURAUD_TEXTURE,
} jrvertexmode_t;

typedef enum {
    JR_CULLMODE_NO_CULLING=0,
    JR_CULLMODE_CULL_POSITIVE,
    JR_CULLMODE_CULL_NEGATIVE
} jrcullmode_t;

typedef enum {
    JR_OUT_XPLUS  = 1,
    JR_OUT_XMINUS = 2,
    JR_OUT_YPLUS  = 4,
    JR_OUT_YMINUS = 8,
    JR_OUT_ZPLUS  =16,
    JR_OUT_ZMINUS =32
} jroutcode_t;

typedef enum {
    JR_HINT_FULLOPACITY   = 1,
    JR_HINT_ONE_BIT_ALPHA = 2,
    JR_HINT_GREYSCALE     = 4
} jrtxthint_t;

typedef enum {
    JR_CS_WORLD = 1,
    JR_CS_EYE
} jrcoordsystem_t;

typedef enum {
    JR_ZBFUNC_NEVER,
    JR_ZBFUNC_ALWAYS,
    JR_ZBFUNC_LESS,
    JR_ZBFUNC_LEQUAL,
    JR_ZBFUNC_EQUAL,
    JR_ZBFUNC_GEQUAL,
    JR_ZBFUNC_GREATER,
    JR_ZBFUNC_NOTEQUAL
} jrzbfunc_t;
    
typedef enum {
    JR_TEXDIM_U,
    JR_TEXDIM_V,
    JR_TEXDIM_W
} jrtexdim_t;
    
typedef enum {
    JR_WRAPMODE_CLAMP,
    JR_WRAPMODE_REPEAT
} jrwrapmode_t;
    
/*
 Fog types, used with setFogType
 */
enum {
    JR_FOGTYPE_LINEAR,
    JR_FOGTYPE_EXP,
    JR_FOGTYPE_EXP_SQUARE,
    JR_FOGTYPE_FARAWAY
};
typedef int jrfogtype_t;

/*
 Blend modes, used with setBlendMode
*/
typedef enum {
    JR_BLENDMODE_BLEND,
    JR_BLENDMODE_ADDITIVE,
    JR_BLENDMODE_SUBTRACTIVE,
    JR_BLENDMODE_MULTIPLICATIVE
} jrblendmode_t;

typedef unsigned int jrtxtid_t;

/* Class JRenderer, the abstract base class for all hardware-specific
 renderers */

class JRenderer
{
public:

    /* Destructor */
    virtual ~JRenderer() { }

    /* Initialisation methods ---------------------------------------*/

    virtual void setVertexMode(jrvertexmode_t mode) = 0;
    virtual void setCoordSystem(jrcoordsystem_t cs) = 0;
    
    virtual void setCullMode(jrcullmode_t mode) = 0;
    virtual void setClipRange(float near, float far) = 0;

    virtual void setCamera(jcamera_t *cam) = 0;

    virtual void setBackgroundColor(const jcolor3_t *col) = 0;
    
    /* Shading methods ----------------------------------------------*/
    virtual void enableSmoothShading() = 0;
    virtual void disableSmoothShading() = 0;

    /* Alpha blending methods ---------------------------------------*/
    virtual void enableAlphaBlending() = 0;
    virtual void disableAlphaBlending() = 0;
    virtual jBool alphaBlendingEnabled() = 0;
    virtual void setBlendMode(jrblendmode_t) = 0;

    /* Z-Buffer methods ---------------------------------------------*/

    virtual void setZBufferFunc(jrzbfunc_t func) = 0;
    virtual void enableZBufferReading() = 0;
    virtual void disableZBufferReading() = 0;
    virtual void enableZBufferWriting() = 0;
    virtual void disableZBufferWriting() = 0;
    virtual void enableZBuffer() = 0;
    virtual void disableZBuffer() = 0;

    /* Gamma correction methods -------------------------------------*/

    virtual float getGammaCorrectionValue() = 0;
    virtual void  setGammaCorrectionValue(float val) = 0;

    /* Drawing methods ----------------------------------------------*/

    virtual void begin(jrdrawmode_t mode) = 0;
    virtual void end() = 0;

    // Legacy functions
    virtual void addVertex(jvertex_col    *v) = 0;
    virtual void addVertex(jvertex_txt    *v) = 0;
    virtual void addVertex(jvertex_coltxt *v) = 0;
    
    // Vertex functions
    virtual void setAlpha(float alpha) = 0;
    virtual void setColor(const Vector &) = 0;
    virtual void setUVW(const Vector &) = 0;
    virtual void setAbsoluteUVW(const Vector &) = 0;
    virtual void setNormal(const Vector &) = 0;
    virtual void vertex(const Vector &) = 0;
    virtual void vertex(const Vector2 &) = 0;
    
    virtual void flush() = 0;

    virtual void clear() = 0;

    /* Texturing methods --------------------------------------------*/

    virtual void enableTexturing() = 0;
    virtual void disableTexturing() = 0;
    virtual bool texturingEnabled() = 0;
    
    virtual unsigned int getMaxCompression(unsigned int hint) = 0;

    virtual jError createTexture(const jsprite_t *sprite,
                                 unsigned int hint,
                                 unsigned int compression,
                                 jBool mipmap,
                                 jrtxtid_t *dst) = 0;

    virtual jError destroyTexture(jrtxtid_t txtid) = 0;

    virtual jError setTexture(jrtxtid_t txtid) = 0;
    
    virtual void setWrapMode(jrtexdim_t dim, jrwrapmode_t mode) = 0;
    
    /* Fogging methods ----------------------------------------------*/

    virtual void   setFogColor(const jcolor3_t *col) = 0;
    virtual void   getFogColor(jcolor3_t *col) = 0;
    virtual jError setFogType(jrfogtype_t type, float density) = 0;
    
    virtual jError enableFog() = 0;
    virtual jError disableFog() = 0;
    virtual jBool  fogEnabled() = 0;
    
    /* Matrix stack -------------------------------------------------*/
    virtual void pushMatrix() = 0;
    virtual void setMatrix(const Matrix &) = 0;
    virtual void multMatrix(const Matrix &) = 0;
    virtual void popMatrix() = 0;
    
    /* Additional clipping planes -----------------------------------*/
    virtual jError pushClipPlane(const Vector & n, float c) = 0;
    virtual void popClipPlanes(int n) = 0;
    
    /* Lighting -----------------------------------------------------*/
    virtual void enableLighting() = 0;
    virtual void disableLighting() = 0;
    virtual void setAmbientColor(const Vector &) = 0;
    
    virtual Ptr<JMaterial> createMaterial() = 0;
    
    virtual Ptr<JPointLight> createPointLight() = 0;
    virtual Ptr<JDirectionalLight> createDirectionalLight() = 0;
    
    /* Convenience operators ----------------------------------------*/
    inline JRenderer & operator<< (const Vector & v) {vertex(v); return *this;}
    inline JRenderer & operator<< (const Vector2 & v) {vertex(v); return *this;}
};


#endif
