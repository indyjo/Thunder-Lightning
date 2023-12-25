#ifndef _JOGI_OPENGL_H
#define _JOGI_OPENGL_H

#include <stack>
#include "JRenderer.h"

#define JGL_MAX_TEXTURES 256

typedef struct {
    int size_w, size_h;
    unsigned int gl_tex_name;
    bool used;
} jgl_texture_t;

class JOpenGLRenderer : public JRenderer
{
public:

    /* Initialisation methods ---------------------------------------*/
    JOpenGLRenderer();

    virtual void resize(int new_width, int new_height);

    virtual void setVertexMode(jrvertexmode_t mode);
    virtual void setCoordSystem(jrcoordsystem_t cs);
    
    virtual void setCullMode(jrcullmode_t mode);
    virtual void setClipRange(float near, float far);

    virtual void setCamera(jcamera_t *cam);

    virtual void setBackgroundColor(const jcolor3_t *col);
    virtual void setClearDepth(float);

    /* Status querying methods --------------------------------------*/
    virtual int getWidth();
    virtual int getHeight();
    virtual float getAspect();
    virtual float getFocus();
    virtual float getClipNear();
    virtual float getClipFar();
    
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
    virtual void setNormal(const Vector &);
    virtual void vertex(const Vector &);
    virtual void vertex(const Vector2 &);

    virtual void flush();

    virtual void clear(bool color, bool depth);

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
    virtual jError createEmptyTexture(  jrtxtformat_t fmt,
                                        int width, int height,
                                        jrtxtid_t *dst);

    virtual jError destroyTexture(jrtxtid_t txtid);

    virtual jError setTexture(jrtxtid_t txtid);
    
    virtual void setWrapMode(jrtexdim_t dim, jrwrapmode_t mode);
    
    virtual unsigned int getGLTexFromTxtid(jrtxtid_t txtid);
#ifndef __EMSCRIPTEN__
    virtual jError createTxtidFromGLTex(unsigned int tex, jrtxtid_t *txtid);
#endif
    
    virtual int getTextureWidth(jrtxtid_t tex);
    virtual int getTextureHeight(jrtxtid_t tex);
    
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
    
    /* Additional clipping planes -----------------------------------*/
    virtual jError pushClipPlane(const Vector & n, float c) ;
    virtual void popClipPlanes(int n);
    
    /* Lighting -----------------------------------------------------*/
    virtual void enableLighting();
    virtual void disableLighting();
    virtual void setAmbientColor(const Vector &);
    
    virtual Ptr<JMaterial> createMaterial();
    
    virtual Ptr<JPointLight> createPointLight();
    virtual Ptr<JDirectionalLight> createDirectionalLight();

private:
    void initProjectionMatrix();
    void initModelViewMatrix();
    void uploadTexture(int level, int format, int w, int h, ju32 *src);
    void setupFog();
    void initTextureArray();
    int findFreeTexture();
    void textureScaleDown(int w, int h, ju32 *src, ju32 *dst);
    
    friend struct JOpenGLLight;
    unsigned int requestLight();
    void releaseLight(unsigned int);

private:
    float clip_near, clip_far;
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
    int clip_planes;
    std::stack<unsigned int> free_lights;
};

struct JOpenGLMaterial : public JMaterial {
	Vector diffuse, specular, ambient, emission;
	float shininess;
	
	JOpenGLMaterial();
	
	virtual void activate();
	
	virtual void setDiffuse(const Vector &);
	virtual void setSpecular(const Vector &);
	virtual void setAmbient(const Vector &);
	virtual void setAmbientAndDiffuse(const Vector &);
	virtual void setEmission(const Vector &);
	
	virtual void setShininess(float);
};


struct JOpenGLLight {
	JOpenGLRenderer *renderer;
	unsigned int gl_name;
	
	JOpenGLLight(JOpenGLRenderer *r);
	~JOpenGLLight();
	void setEnabled(bool e);
	bool getEnabled();
	void setColor(const Vector &);
	void setAttenuation(float squared, float linear, float constant);
	void setPosition(const Vector &);
	void setDirection(const Vector &);
};

struct JOpenGLPointLight: public JPointLight, public JOpenGLLight {
	JOpenGLPointLight(JOpenGLRenderer *);
	
	virtual void setEnabled(bool);
	virtual bool getEnabled();
	virtual void setColor(const Vector &);
	virtual void setAttenuation(float squared, float linear, float constant);
	virtual void setPosition(const Vector &);
};
	
struct JOpenGLDirectionalLight: public JDirectionalLight, public JOpenGLLight {
	JOpenGLDirectionalLight(JOpenGLRenderer *);
	
	virtual void setEnabled(bool);
	virtual bool getEnabled();
	virtual void setColor(const Vector &);
	virtual void setDirection(const Vector &);
};	

#endif
