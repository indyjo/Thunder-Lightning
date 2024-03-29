#include <cstdio>
#include <cstdlib>
#include <cstring>
#if defined(__MINGW32__) || defined(_MSC_VER)
#include <windows.h>
#endif
#ifdef HAVE_REGAL
#include <GL/Regal.h>
#include <GL/RegalGLU.h>
#else
#include <gl.h>
#include <glu.h>
#endif
#include "jogi.h"
#include "JOpenGLRenderer.h"

// Default projection and clipping values
#define DEFAULT_ASPECT 1.3
#define CLIP_NEAR 1.0
#define CLIP_FAR 5000.0

#include <tnl.h>
JOpenGLRenderer::JOpenGLRenderer()
: clip_planes(0)
{
    ls_message("<JOpenGLRenderer::JOpenGLRenderer)>\n");

#ifdef HAVE_CEGUI
    // This is an ugly hack to force linking to GLU.
    // Why would we want to do that? Because CEGUIOpenGLRenderer depends on a
    // (one!) symbol of libGLU and doesn't link in libGLU by itself.
    // If we don't depend on some symbol of libGLU, it won't be linked in
    // and CEGUIOpenGLRender won't be able to satisfy its dependencies.
    // So we perform some random NOP call to GLU.
    gluErrorString(GLU_OUT_OF_MEMORY);
#endif

    coord_sys=JR_CS_WORLD;

    camera.init();
    clip_near = CLIP_NEAR;
    clip_far = CLIP_FAR;

    resize(256, 256);
    
    initModelViewMatrix();

    setCullMode(JR_CULLMODE_NO_CULLING);

    ls_message("Initializing JOpenGLRenderer.\n");
    ls_message("Some information about GL implementation:\n");
    ls_message("Vendor:%s\n", glGetString(GL_VENDOR));
    ls_message("Renderer:%s\n", glGetString(GL_RENDERER));
    ls_message("Version:%s\n", glGetString(GL_VERSION));
    ls_message("Extensions:%s\n", glGetString(GL_EXTENSIONS));

    glClearColor(0.0,0.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    alpha=1.0;
    color=Vector(1,1,1);
    uvw=Vector(0,0,0);

    setZBufferFunc(JR_ZBFUNC_LESS);
    enableZBuffer();

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    //glEnable(GL_BLEND);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    initTextureArray();
    current_tex=0;

    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glHint(GL_FOG_HINT, GL_NICEST);
    
    int max_lights = std::min(GL_MAX_LIGHTS, 64);
    while (max_lights--) {
    	free_lights.push(GL_LIGHT0 + max_lights);
    }
    ls_message("</JOpenGLRenderer::JOpenGLRenderer>\n");
}

void JOpenGLRenderer::resize(int new_width, int new_height)
{
    glViewport(0,0, new_width, new_height);
}

void JOpenGLRenderer::setVertexMode(jrvertexmode_t mode)
{
    switch (mode) {
    case JR_VERTEXMODE_GOURAUD:
        enableSmoothShading();
        disableTexturing();
        break;
    case JR_VERTEXMODE_TEXTURE:
        disableSmoothShading();
        enableTexturing();
        glColor3f(1.0,1.0,1.0);
        break;
    case JR_VERTEXMODE_GOURAUD_TEXTURE:
        enableSmoothShading();
        enableTexturing();
        break;
    }
}

void JOpenGLRenderer::setCullMode(jrcullmode_t mode)
{
    switch(mode) {
    case JR_CULLMODE_NO_CULLING:
        glDisable(GL_CULL_FACE);
        break;
    case JR_CULLMODE_CULL_POSITIVE:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        break;
    case JR_CULLMODE_CULL_NEGATIVE:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        break;
    }
}

void JOpenGLRenderer::setClipRange(float cnear, float cfar)
{
    clip_near = cnear;
    clip_far = cfar;
    initProjectionMatrix();
    setupFog();
}

void JOpenGLRenderer::setCamera(jcamera_t *cam)
{
    camera.init(cam);
    initModelViewMatrix();
    initProjectionMatrix();
}

void JOpenGLRenderer::setBackgroundColor(const jcolor3_t *col)
{
    glClearColor(col->r / 256.0, col->g / 256.0, col->b / 256.0, 0.0);
}

void JOpenGLRenderer::setClearDepth(float val) {
    glClearDepth(val);
}

// --------------------------------------------------------------------------
// Status querying methods
// --------------------------------------------------------------------------

int JOpenGLRenderer::getWidth() {
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    return vp[2];
}
int JOpenGLRenderer::getHeight() {
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    return vp[3];
}
float JOpenGLRenderer::getAspect() {
    return camera.cam.aspect;
}
float JOpenGLRenderer::getFocus() {
    return camera.cam.focus;
}
float JOpenGLRenderer::getClipNear() {
    return clip_near;
}
float JOpenGLRenderer::getClipFar() {
    return clip_far;
}

// --------------------------------------------------------------------------
// Shading methods
// --------------------------------------------------------------------------

void JOpenGLRenderer::enableSmoothShading()
{
    glShadeModel(GL_SMOOTH);
}

void JOpenGLRenderer::disableSmoothShading()
{
    glShadeModel(GL_FLAT);
}

void JOpenGLRenderer::begin(jrdrawmode_t mode)
{
    switch (mode) {
    case JR_DRAWMODE_POINTS:
        glBegin(GL_POINTS);
        break;
    case JR_DRAWMODE_LINES:
        glBegin(GL_LINES);
        break;
    case JR_DRAWMODE_CONNECTED_LINES:
        glBegin(GL_LINE_STRIP);
        break;
    case JR_DRAWMODE_TRIANGLES:
        glBegin(GL_TRIANGLES);
        break;
    case JR_DRAWMODE_TRIANGLE_STRIP:
        glBegin(GL_TRIANGLE_STRIP);
        break;
    case JR_DRAWMODE_TRIANGLE_FAN:
        glBegin(GL_TRIANGLE_FAN);
        break;
    case JR_DRAWMODE_QUADS:
        glBegin(GL_QUADS);
        break;
    }
}

void JOpenGLRenderer::end()
{
    glEnd();
}

void JOpenGLRenderer::addVertex(jvertex_col *v)
{
    setColor(Vector(v->col.r / 256, v->col.g / 256, v->col.b / 256));
    vertex( Vector(v->p.x, v->p.y, v->p.z) );
}

void JOpenGLRenderer::addVertex(jvertex_txt *v)
{
    setColor(Vector(1,1,1));
    setAbsoluteUVW( Vector(v->txt.x, v->txt.y, 0) );
    vertex( Vector(v->p.x, v->p.y, v->p.z) );
}

void JOpenGLRenderer::addVertex(jvertex_coltxt *v)
{
    setColor(Vector(v->col.r / 256, v->col.g / 256, v->col.b / 256));
    setAbsoluteUVW( Vector(v->txt.x, v->txt.y, 0) );
    vertex( Vector(v->p.x, v->p.y, v->p.z) );
}

void JOpenGLRenderer::setAlpha(float new_alpha)
{
    alpha=new_alpha;
}

void JOpenGLRenderer::setColor(const Vector & col) {
    color=col;
}

void JOpenGLRenderer::setUVW(const Vector & u) {
    uvw=u;
}

void JOpenGLRenderer::setAbsoluteUVW(const Vector & u) {
    uvw = u;
    uvw[0] /= texture[current_tex].size_w;
    uvw[1] /= texture[current_tex].size_h;
}

void JOpenGLRenderer::setNormal(const Vector &v) {
	glNormal3f(v[0],v[1],v[2]);
}


void JOpenGLRenderer::vertex(const Vector & v) {
    glColor4f(color[0], color[1], color[2], alpha);
    glTexCoord2f(uvw[0], uvw[1]);
    glVertex3f(v[0],v[1],v[2]);
}

void JOpenGLRenderer::vertex(const Vector2 & v) {
    glColor4f(color[0], color[1], color[2], alpha);
    glTexCoord2f(uvw[0], uvw[1]);
    glVertex2f(v[0],v[1]);
}

void JOpenGLRenderer::flush()
{
    glFlush();
}

unsigned int JOpenGLRenderer::getMaxCompression(unsigned int hint)
{
    return 0;
}

void JOpenGLRenderer::enableTexturing()
{
    glEnable(GL_TEXTURE_2D);
}

void JOpenGLRenderer::disableTexturing()
{
    glDisable(GL_TEXTURE_2D);
}

bool JOpenGLRenderer::texturingEnabled()
{
    return glIsEnabled(GL_TEXTURE_2D);
}

jError JOpenGLRenderer::createTexture(const jsprite_t *sprite,
                                unsigned int hint,
                                unsigned int compression,
                                jBool mipmap,
                                jrtxtid_t *dst)
{
    int tex;
    GLuint tex_name;
    GLint tex_format=GL_RGBA;

    glGenTextures(1, &tex_name);
    glBindTexture(GL_TEXTURE_2D, tex_name);

    tex=findFreeTexture();
    if (tex==-1) {
        return JERR_NOT_ENOUGH_MEMORY;
    }
    *dst=tex;
    texture[tex].used=true;
    texture[tex].gl_tex_name=tex_name;
    texture[tex].size_w=sprite->w;
    texture[tex].size_h=sprite->h;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (mipmap) {
        glTexParameteri(GL_TEXTURE_2D,
                GL_TEXTURE_MIN_FILTER,
                GL_LINEAR_MIPMAP_NEAREST);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    // Choose OpenGL texture format depending on the hint argument
    // Emscripten apparently doesn't support any format except GL_RGBA.
#ifndef __EMSCRIPTEN__
    if (hint & JR_HINT_FULLOPACITY) {
        if (hint & JR_HINT_GREYSCALE) {
            tex_format = GL_LUMINANCE;
        } else {
            tex_format = GL_RGB;
        }
    } else {
        if (hint & JR_HINT_ONE_BIT_ALPHA) {
            if (hint & JR_HINT_GREYSCALE) {
                tex_format = GL_LUMINANCE6_ALPHA2;
            } else {
                tex_format = GL_RGB5_A1;
            }
        } else {
            if (hint & JR_HINT_GREYSCALE) {
                tex_format = GL_LUMINANCE_ALPHA;
            } else {
                tex_format = GL_RGBA;
            }
        }
    }
#endif

    if (mipmap) {
        int w=sprite->w, h=sprite->h;
        ju32 *buf = (ju32*) malloc(4*w*h);
        memcpy(buf, sprite->buf, 4*w*h);
        for (int level=0; w>0 && h>0; w/=2, h/=2, level++)
        {
            uploadTexture(level, tex_format, w, h, buf);
            textureScaleDown(w, h, buf, buf);
        }
        free(buf);
    } else {
        uploadTexture(0, tex_format, sprite->w, sprite->h, sprite->buf);
    }

    return JERR_OK;
}

jError JOpenGLRenderer::createEmptyTexture(jrtxtformat_t format, int width, int height, jrtxtid_t *dst)
{
    int tex;
    GLuint tex_name;

    glGenTextures(1, &tex_name);
    glBindTexture(GL_TEXTURE_2D, tex_name);

    tex=findFreeTexture();
    if (tex==-1) {
        return JERR_NOT_ENOUGH_MEMORY;
    }
    *dst=tex;
    texture[tex].used=true;
    texture[tex].gl_tex_name=tex_name;
    texture[tex].size_w=width;
    texture[tex].size_h=height;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    GLenum gl_format;
    switch( format ) {
    case JR_FORMAT_RGB:
        gl_format = GL_RGB;
        break;
    case JR_FORMAT_RGBA:
        gl_format = GL_RGBA;
        break;
    case JR_FORMAT_GREYSCALE:
        gl_format = GL_LUMINANCE;
        break;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, gl_format,
        width, height, 0,
        GL_RGB, GL_UNSIGNED_BYTE,
        NULL);

    return JERR_OK;
}

jError JOpenGLRenderer::destroyTexture(jrtxtid_t txtid)
{
    texture[txtid].used=false;
    glDeleteTextures(1,(GLuint *) &texture[txtid].gl_tex_name);
    return JERR_OK;
}

jError JOpenGLRenderer::setTexture(jrtxtid_t txtid)
{
    glBindTexture(GL_TEXTURE_2D, texture[txtid].gl_tex_name);
    current_tex=txtid;
    return JERR_OK;
}

void JOpenGLRenderer::setWrapMode( jrtexdim_t dim, jrwrapmode_t mode)
{
    GLenum pname;
    int param;

    if (dim == JR_TEXDIM_U) pname = GL_TEXTURE_WRAP_S;
    else pname = GL_TEXTURE_WRAP_T;

    if (mode == JR_WRAPMODE_REPEAT) param = GL_REPEAT;
    else param = GL_CLAMP_TO_EDGE;

    glTexParameteri(GL_TEXTURE_2D, pname, param);
}

unsigned int JOpenGLRenderer::getGLTexFromTxtid(jrtxtid_t txtid)
{
    return (unsigned int) texture[txtid].gl_tex_name;
}

#ifndef __EMSCRIPTEN__
jError JOpenGLRenderer::createTxtidFromGLTex(unsigned int gltex, jrtxtid_t *txtid)
{
    int tex=findFreeTexture();
    if (tex==-1) {
        return JERR_NOT_ENOUGH_MEMORY;
    }

    *txtid=(jrtxtid_t) tex;
    texture[tex].used=true;
    texture[tex].gl_tex_name=gltex;
    
    glPushAttrib(GL_TEXTURE_BIT);
    glBindTexture(GL_TEXTURE_2D, gltex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texture[tex].size_w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texture[tex].size_h);
    glPopAttrib();
    return JERR_OK;
}
#endif

int JOpenGLRenderer::getTextureWidth(jrtxtid_t tex)
{
    return texture[tex].size_w;
}
int JOpenGLRenderer::getTextureHeight(jrtxtid_t tex)
{
    return texture[tex].size_h;
}

void JOpenGLRenderer::setFogColor(const jcolor3_t *col)
{
    fog_color[0]=col->r / 256.0;
    fog_color[1]=col->g / 256.0;
    fog_color[2]=col->b / 256.0;
    fog_color[3]=1.0;
    glFogfv(GL_FOG_COLOR, fog_color);
}

void JOpenGLRenderer::getFogColor(jcolor3_t *col)
{
    col->r=fog_color[0] * 256.0;
    col->g=fog_color[1] * 256.0;
    col->b=fog_color[2] * 256.0;
}

jError JOpenGLRenderer::setFogType(jrfogtype_t type, float density)
{
    fog_type=type;
    fog_density=density;

    setupFog();

    return JERR_OK;
}

jError JOpenGLRenderer::enableFog()
{
    glEnable(GL_FOG);
    return JERR_OK;
}

jError JOpenGLRenderer::disableFog()
{
    glDisable(GL_FOG);
    return JERR_OK;
}

jBool JOpenGLRenderer::fogEnabled()
{
    return glIsEnabled(GL_FOG);
}

void JOpenGLRenderer::pushMatrix()
{
    glPushMatrix();
}

void JOpenGLRenderer::popMatrix()
{
    glPopMatrix();
}

void JOpenGLRenderer::setMatrix(const Matrix & M)
{
    glLoadMatrixf(M.glMatrix());
}

void JOpenGLRenderer::multMatrix(const Matrix & M)
{
    glMultMatrixf(M.glMatrix());
}

jError JOpenGLRenderer::pushClipPlane(const Vector & n, float c) {
    // Check whether OpenGL implementation supports another addidtional clip plane
    int max_clip_planes=0;
    glGetIntegerv(GL_MAX_CLIP_PLANES, &max_clip_planes);
    if (clip_planes == max_clip_planes) return JERR_NOT_SUPPORTED;
    
    // Enable the plane
    double plane[4] = {n[0],n[1],n[2],c};
    glClipPlane(GL_CLIP_PLANE0 + clip_planes, plane);
    glEnable(GL_CLIP_PLANE0 + clip_planes);
    ++clip_planes;
    
    return JERR_OK;
}

void JOpenGLRenderer::popClipPlanes(int n) {
    if (n>clip_planes) return;
    // disable n clipping planes
    clip_planes -= n;
    while (n--) {
        glDisable(GL_CLIP_PLANE0 + n);
    }
}


void JOpenGLRenderer::setZBufferFunc(jrzbfunc_t func)
{
    switch(func) {
    case JR_ZBFUNC_NEVER:
        glDepthFunc(GL_NEVER);
        break;
    case JR_ZBFUNC_ALWAYS:
        glDepthFunc(GL_ALWAYS);
        break;
    case JR_ZBFUNC_LESS:
        glDepthFunc(GL_LESS);
        break;
    case JR_ZBFUNC_LEQUAL:
        glDepthFunc(GL_LEQUAL);
        break;
    case JR_ZBFUNC_EQUAL:
        glDepthFunc(GL_EQUAL);
        break;
    case JR_ZBFUNC_GEQUAL:
        glDepthFunc(GL_GEQUAL);
        break;
    case JR_ZBFUNC_GREATER:
        glDepthFunc(GL_GREATER);
        break;
    case JR_ZBFUNC_NOTEQUAL:
        glDepthFunc(GL_NOTEQUAL);
        break;
    }
}

void JOpenGLRenderer::enableZBufferReading() {
     glEnable(GL_DEPTH_TEST);
}

void JOpenGLRenderer::disableZBufferReading() {
     glDisable(GL_DEPTH_TEST);
}

void JOpenGLRenderer::enableZBufferWriting() {
    glDepthMask(GL_TRUE);
}

void JOpenGLRenderer::disableZBufferWriting() {
    glDepthMask(GL_FALSE);
}

void JOpenGLRenderer::enableZBuffer()
{
     glEnable(GL_DEPTH_TEST);
     glDepthMask(GL_TRUE);
}

void JOpenGLRenderer::disableZBuffer()
{
     glDisable(GL_DEPTH_TEST);
     glDepthMask(GL_FALSE);
}

void JOpenGLRenderer::setCoordSystem(jrcoordsystem_t cs)
{
    coord_sys = cs;
    initModelViewMatrix();
}

float JOpenGLRenderer::getGammaCorrectionValue()
{
    return 1.0f;
}

void JOpenGLRenderer::setGammaCorrectionValue(float)
{
}

void JOpenGLRenderer::enableAlphaBlending()
{
    glEnable(GL_BLEND);
    //glDisable(GL_BLEND);
}

void JOpenGLRenderer::disableAlphaBlending()
{
    glColor4f(1.0, 1.0, 1.0, 1.0);
    alpha = 1.0;
    glDisable(GL_BLEND);
}

jBool JOpenGLRenderer::alphaBlendingEnabled()
{
    return glIsEnabled(GL_BLEND);
}

void JOpenGLRenderer::setBlendMode(jrblendmode_t mode) {
    GLenum sfactor, dfactor;

    switch (mode) {
    case JR_BLENDMODE_BLEND:
        sfactor=GL_SRC_ALPHA;
        dfactor=GL_ONE_MINUS_SRC_ALPHA;
        break;
    case JR_BLENDMODE_ADDITIVE:
        sfactor=GL_SRC_ALPHA;
        dfactor=GL_ONE;
        break;
    case JR_BLENDMODE_SUBTRACTIVE:
        sfactor=GL_ONE_MINUS_SRC_ALPHA;
        dfactor=GL_ONE_MINUS_SRC_ALPHA;
        break;
    case JR_BLENDMODE_MULTIPLICATIVE:
        sfactor=GL_ZERO;
        dfactor=GL_SRC_COLOR;
        break;
    }
    glBlendFunc(sfactor, dfactor);
}

void JOpenGLRenderer::clear(bool color, bool depth) {
    glClear( (color?GL_COLOR_BUFFER_BIT:0) | (depth?GL_DEPTH_BUFFER_BIT:0) );
}

void JOpenGLRenderer::enableLighting() {
	glEnable(GL_LIGHTING);
}

void JOpenGLRenderer::disableLighting() {
	glDisable(GL_LIGHTING);
}

void JOpenGLRenderer::setAmbientColor(const Vector & c) {
	float ambient[] = {c[0], c[1], c[2], 1};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
}

Ptr<JMaterial> JOpenGLRenderer::createMaterial() {
	return new JOpenGLMaterial;
}

Ptr<JPointLight> JOpenGLRenderer::createPointLight() {
	if (free_lights.empty()) return 0;
	return new JOpenGLPointLight(this);
}

Ptr<JDirectionalLight> JOpenGLRenderer::createDirectionalLight() {
	if (free_lights.empty()) return 0;
	return new JOpenGLDirectionalLight(this);
}


/* --------------------- Private Functions -------------------------------- */

void JOpenGLRenderer::initProjectionMatrix() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(
        (-camera.cam.aspect/camera.cam.focus)*clip_near,
        ( camera.cam.aspect/camera.cam.focus)*clip_near,
        (                -1/camera.cam.focus)*clip_near,
        (                 1/camera.cam.focus)*clip_near,
        clip_near, clip_far);
    glMatrixMode(GL_MODELVIEW);
}

void JOpenGLRenderer::initModelViewMatrix()
{
    float m[16];

    glMatrixMode(GL_MODELVIEW);

    if (coord_sys == JR_CS_WORLD) {
        /*m[ 0] = camera.cam.matrix.m[0][0];
        m[ 1] = camera.cam.matrix.m[0][1];
        m[ 2] = camera.cam.matrix.m[0][2];
        m[ 3] = 0.0;

        m[ 4] = camera.cam.matrix.m[1][0];
        m[ 5] = camera.cam.matrix.m[1][1];
        m[ 6] = camera.cam.matrix.m[1][2];
        m[ 7] = 0.0;

        m[ 8] = camera.cam.matrix.m[2][0];
        m[ 9] = camera.cam.matrix.m[2][1];
        m[10] = camera.cam.matrix.m[2][2];
        m[11] = 0.0;*/

        m[ 0] = camera.cam.matrix.m[0][0];
        m[ 1] = camera.cam.matrix.m[1][0];
        m[ 2] = -camera.cam.matrix.m[2][0];
        m[ 3] = 0.0;

        m[ 4] = camera.cam.matrix.m[0][1];
        m[ 5] = camera.cam.matrix.m[1][1];
        m[ 6] = -camera.cam.matrix.m[2][1];
        m[ 7] = 0.0;

        m[ 8] = camera.cam.matrix.m[0][2];
        m[ 9] = camera.cam.matrix.m[1][2];
        m[10] = -camera.cam.matrix.m[2][2];
        m[11] = 0.0;

        m[12] = camera.cam.matrix.m[0][3];
        m[13] = camera.cam.matrix.m[1][3];
        m[14] = -camera.cam.matrix.m[2][3];
        m[15] = 1.0;

        glLoadMatrixf(m);
    } else {
        glLoadIdentity();
        glScalef(1.0,1.0,-1.0);
    }
}


void JOpenGLRenderer::uploadTexture(
        int level, int format, int w, int h, ju32 *src)
{
    ju32 *dst=new ju32[w*h];
    int i;

    for(i=0;i<w*h;i++) {
        dst[i]=
            (src[i] & 0xff000000) |         /* Alpha */
            ((src[i] & 0x00ff0000) >> 16) | /* Blue  */
            (src[i] & 0x0000ff00) |         /* Green */
            ((src[i] & 0x000000ff) << 16);  /* Red   */
    }
    glTexImage2D(GL_TEXTURE_2D, level, format,
        w, h, 0,
        GL_RGBA, GL_UNSIGNED_BYTE,
        dst);
    delete[] dst;
}


void JOpenGLRenderer::setupFog()
{
    if (!glIsEnabled(GL_FOG)) return;
    switch(fog_type) {
    case JR_FOGTYPE_LINEAR:
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogf(GL_FOG_START, clip_near);
        glFogf(GL_FOG_END, clip_far);
        break;
    case JR_FOGTYPE_EXP:
        glFogi(GL_FOG_MODE, GL_EXP);
        glFogf(GL_FOG_DENSITY, fog_density);
        glFogf(GL_FOG_START, clip_near);
        glFogf(GL_FOG_END, clip_far);
        break;
    case JR_FOGTYPE_EXP_SQUARE:
        glFogi(GL_FOG_MODE, GL_EXP2);
        glFogf(GL_FOG_DENSITY, fog_density);
        glFogf(GL_FOG_START, clip_near);
        glFogf(GL_FOG_END, clip_far);
        break;
    case JR_FOGTYPE_FARAWAY:
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogf(GL_FOG_START, 0.25f*clip_near + 0.75f*clip_far);
        glFogf(GL_FOG_END, clip_far);
    }
}

void JOpenGLRenderer::initTextureArray()
{
    for (int i=0; i < JGL_MAX_TEXTURES; i++) {
        texture[i].used=false;
    }
}

int JOpenGLRenderer::findFreeTexture()
{
    for (int i=0; i < JGL_MAX_TEXTURES; i++) {
        if (!texture[i].used) {
            return i;
        }
    }

    return -1;
}

/*
Taken from JGlideRenderer with some minor changes
It should be possible to use the same buffer for src and dst
*/
void JOpenGLRenderer::textureScaleDown(int w, int h, ju32 *src, ju32 *dst)
{
    int r,g,b,a;
    ju32 csrc;
    int x,y;
    int w_half=w/2;

    for (y=0; y<h; y+=2) {
        for (x=0; x<w; x+=2) {
            a=r=g=b=0;
            for (int v=0; v<2; v++) {
                for (int u=0; u<2; u++) {
                    csrc=src[(y+v)*w+x+u];
                    a+=(csrc & 0xff000000) >> 24;
                    r+=(csrc & 0x00ff0000) >> 16;
                    g+=(csrc & 0x0000ff00) >>  8;
                    b+=(csrc & 0x000000ff);
                }
            }

            a/=4;
            r/=4;
            g/=4;
            b/=4;

            dst[(y/2)*w_half+(x/2)]= (a<<24) | (r<<16) | (g<<8) | b;
        }
    }
}

unsigned int JOpenGLRenderer::requestLight() {
	unsigned int light = free_lights.top();
	free_lights.pop();
	return light;
}

void JOpenGLRenderer::releaseLight(unsigned int light) {
	free_lights.push(light);
}


/* ----------------- Functions of related classes ------------------------- */

JOpenGLLight::JOpenGLLight(JOpenGLRenderer *r)
:	renderer(r),
	gl_name(r->requestLight())
{
	glEnable(gl_name);
}
	
JOpenGLLight::~JOpenGLLight() {
	glDisable(gl_name);
	renderer->releaseLight(gl_name);
}

void JOpenGLLight::setEnabled(bool e) {
	if (e) glEnable(gl_name);
	else   glDisable(gl_name);
}

bool JOpenGLLight::getEnabled() {
	return glIsEnabled(gl_name);
}

void JOpenGLLight::setColor(const Vector &c) {
	float val[4] = {c[0], c[1], c[2], 1};
	glLightfv(gl_name, GL_DIFFUSE, val);
	glLightfv(gl_name, GL_SPECULAR, val);
}
	
void JOpenGLLight::setAttenuation(float squared, float linear, float constant) {
	glLightfv(gl_name, GL_CONSTANT_ATTENUATION, &constant);
	glLightfv(gl_name, GL_LINEAR_ATTENUATION, &linear);
	glLightfv(gl_name, GL_QUADRATIC_ATTENUATION, &squared);	
}

void JOpenGLLight::setPosition(const Vector &p) {
	float val[] = {p[0],p[1],p[2],1};
	glLightfv(gl_name, GL_POSITION, val);	
}
	
void JOpenGLLight::setDirection(const Vector &p) {
	float val[] = {p[0],p[1],p[2],0};
	glLightfv(gl_name, GL_POSITION, val);	
}

JOpenGLPointLight::JOpenGLPointLight(JOpenGLRenderer *r)
:	JOpenGLLight(r)
{ }

void JOpenGLPointLight::setEnabled(bool b) { JOpenGLLight::setEnabled(b); }
bool JOpenGLPointLight::getEnabled() { return JOpenGLLight::getEnabled(); }
void JOpenGLPointLight::setColor(const Vector &c) { JOpenGLLight::setColor(c); }
void JOpenGLPointLight::setAttenuation(float squared, float linear, float constant)
{	JOpenGLLight::setAttenuation(squared, linear, constant); }
void JOpenGLPointLight::setPosition(const Vector &p) { JOpenGLLight::setPosition(p); }

JOpenGLDirectionalLight::JOpenGLDirectionalLight(JOpenGLRenderer *r)
:	JOpenGLLight(r)
{ }

void JOpenGLDirectionalLight::setEnabled(bool b) { JOpenGLLight::setEnabled(b); }
bool JOpenGLDirectionalLight::getEnabled() { return JOpenGLLight::getEnabled(); }
void JOpenGLDirectionalLight::setColor(const Vector &c) { JOpenGLLight::setColor(c); }
void JOpenGLDirectionalLight::setDirection(const Vector &d) { JOpenGLLight::setDirection(d); }


JOpenGLMaterial::JOpenGLMaterial()
:	diffuse(.8,.8,.8), specular(0,0,0), ambient(.2,.2,.2), emission(0,0,0),
	shininess(16)
{ }

void JOpenGLMaterial::activate() {
	float param[][4] = {
		{diffuse[0],diffuse[1],diffuse[2],1},
		{specular[0],specular[1],specular[2],1},
		{ambient[0],ambient[1],ambient[2],1},
		{emission[0],emission[1],emission[2],1}};
#ifndef __EMSCRIPTEN__
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, param[0]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, param[1]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, param[2]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, param[3]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &shininess);
#endif
};

void JOpenGLMaterial::setDiffuse(const Vector &c) { diffuse = c; }
void JOpenGLMaterial::setSpecular(const Vector &c) { specular = c; }
void JOpenGLMaterial::setAmbient(const Vector &c) { ambient = c; }
void JOpenGLMaterial::setAmbientAndDiffuse(const Vector &c) { ambient = diffuse = c; }
void JOpenGLMaterial::setEmission(const Vector &c) { emission = c; }
void JOpenGLMaterial::setShininess(float f) { shininess = f; }
