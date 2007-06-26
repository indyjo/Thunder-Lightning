#include <algorithm>

#include <GL/gl.h>

#include <interfaces/ICamera.h>
#include <interfaces/IGame.h>
#include <modules/jogi/JRenderer.h>
#include <debug.h>
#include "RenderPass.h"

RenderPass::RenderPass(WeakPtr<RenderPassList> rpl)
    : renderpasslist(rpl)
    , enabled(false)
    , context_enabled(false)
    , rendertotex_enabled(false)
    , clear_depth_enabled(true)
    , clear_color_enabled(false)
    , width(256)
    , height(256)
    , tex_needs_update(false)
    , background_color(Vector(0,0,0))
{
}

RenderPass::~RenderPass()
{ }

void RenderPass::setEnabled(bool b) { enabled = b; }
bool RenderPass::isEnabled() { return enabled; }
    
void RenderPass::dependsOn(Ptr<RenderPass> other) {
    Ptr<RenderPassList> rpl = renderpasslist.lock();
    if (rpl) {
        rpl->move(this, other);
    }
}
    
void RenderPass::setResolution(int resx, int resy) {
    width = resx;
    height = resy;
    
    if (rendertotex_enabled) tex_needs_update = true;
}
int RenderPass::getWidth() { return width; }
int RenderPass::getHeight() { return height; }
    
void RenderPass::setBackgroundColor(const Vector& c) { background_color = c; }
const Vector& RenderPass::getBackgroundColor() { return background_color; }
    
void RenderPass::enableClearDepth(bool b) { clear_depth_enabled = b; }
bool RenderPass::isClearDepthEnabled() { return clear_depth_enabled; }
    
void RenderPass::enableClearColor(bool b) { clear_color_enabled = b; }
bool RenderPass::isClearColorEnabled() { return clear_color_enabled; }
    
void RenderPass::setRenderToTexture(bool b) {
    rendertotex_enabled = b;
    if (b) {
        if (!tex) tex_needs_update = true;
    } else {
        tex = 0;
        tex_needs_update = false;
    }
}
bool RenderPass::isRenderToTexture() { return rendertotex_enabled; }

Ptr<Texture> RenderPass::getTexture() {
    if (tex_needs_update) {
        createTex();
    }
    return tex;
}
    
void RenderPass::setRenderContext(const RenderContext &ctx) {
    context = ctx;
}

RenderContext& RenderPass::getRenderContext() { return context; }
void RenderPass::setRenderContextEnabled(bool b) { context_enabled = b; }
bool RenderPass::isRenderContextEnabled() { return context_enabled; }
    
RenderPass::RenderSignal & RenderPass::preScene() { return pre_scene; }
RenderPass::RenderSignal & RenderPass::postScene() { return post_scene; }
    
void RenderPass::beginRender(JRenderer* r) {
    r->resize(width, height);
    if (clear_color_enabled) {
        jcolor3_t col;
        col.r = 255*background_color[0];
        col.g = 255*background_color[1];
        col.b = 255*background_color[2];
        r->setBackgroundColor(&col);
    }
    r->clear(clear_color_enabled, clear_depth_enabled);
    preScene().emit(this);
}

void RenderPass::endRender(JRenderer* r) {
    postScene().emit(this);
    
    if (tex_needs_update) {
        createTex();
    }
    
    if (rendertotex_enabled) {
        glReadBuffer(GL_BACK);
        r->setTexture(tex->getTxtid());
        glCopyTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGB,
            0,0,width,height,
            0);
    }
}

void RenderPass::createTex() {
    Ptr<RenderPassList> rpl = renderpasslist.lock();
    if (!rpl) return;
    Ptr<IGame> game = rpl->thegame.lock();
    if (!game) return;
    JRenderer *renderer = game->getRenderer();
    
    jrtxtid_t txtid;
    renderer->createEmptyTexture(JR_FORMAT_RGB, width, height, &txtid);
    tex = new Texture(txtid, *renderer);
    renderer->setTexture(tex->getTxtid());
    renderer->setWrapMode(JR_TEXDIM_U, JR_WRAPMODE_CLAMP);
    renderer->setWrapMode(JR_TEXDIM_V, JR_WRAPMODE_CLAMP);
        
    tex_needs_update = false;
}

RenderPassList::RenderPassList(WeakPtr<IGame> game)
    : thegame(game)
{ }

RenderPassList::~RenderPassList() { }

Ptr<RenderPass> RenderPassList::createRenderPass() {
    Ptr<RenderPass> pass = new RenderPass(this);
    render_passes.push_front(pass);
    return pass;
}

/// Do the actual rendering of the passes
void RenderPassList::renderPasses() {
    Ptr<IGame> game = thegame.lock();
    if (!game) return;
    
    typedef Passes::iterator Iter;
    Iter i=render_passes.begin();
    while(i != render_passes.end()) {
        Ptr<RenderPass> pass = i->lock();
        if (!pass) {
            ls_message("Removing destroyed render pass.\n");
            // remove deleted renderpasses from list
            i = render_passes.erase(i);
            continue;
        }
        
        if (pass->isEnabled()) {
            pass->beginRender(game->getRenderer());
            if (pass->isRenderContextEnabled()) {
                game->renderWithContext(&pass->getRenderContext());
            }
            pass->endRender(game->getRenderer());
        }
        ++i;
    }
}

namespace {
    bool isRenderToTexture(WeakPtr<RenderPass> wrp) {
        Ptr<RenderPass> rp = wrp.lock();
        if (rp) return rp->isRenderToTexture();
        else return false;
    }
} // namespace

void RenderPassList::drawMosaic() {
    int ntex = std::count_if(render_passes.begin(), render_passes.end(), isRenderToTexture);
    // rows*cols=ntex
    // cols:rows=3:2
    // 2*cols = 3*rows --> cols = 3/2 * rows
    // --> rows = ntex:cols = 2/3 * ntex : rows
    int rows = std::max(2, int(sqrt(2.0*ntex/3)));
    int cols = std::max(3, int((3*rows+1)/2));
    
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    int vp_width = vp[2];
    int vp_height = vp[3];
    
    int sz = std::max(vp_width / cols, vp_height / rows) * 30 / 100;
    

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    glDisable(GL_CULL_FACE);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, vp_width, 0, vp_height, -2, 2);
    glMatrixMode(GL_MODELVIEW);
    
    glPushMatrix();
    glLoadIdentity();
    
    typedef Passes::iterator Iter;
    int idx = 0;
    for( Iter i=render_passes.begin(); i!= render_passes.end(); ++i) {
        Ptr<RenderPass> pass = i->lock();
        if (!pass) continue;
        if (!pass->isRenderToTexture()) continue;
        
        int r = idx / cols;
        int c = idx % cols;
        int center_x = (2*c+1) * vp_width / cols / 2;
        int center_y = (2*r+1) * vp_height / rows / 2;

        ls_message("Mosaic tile. r=%d c=%d center_x=%d center_y=%d sz=%d\n",
            r,c,center_x,center_y,sz);
        
        GLuint tex = pass->getTexture()->getGLTex();
        
        glColor3f(1,1,1);

        glBindTexture(GL_TEXTURE_2D, tex);
        glEnable(GL_TEXTURE_2D);
        
        
        glBegin(GL_QUADS);
        
        glTexCoord2f(0,0);
        glVertex2i( center_x - sz, center_y - sz);
        glTexCoord2f(0,1);
        glVertex2i( center_x - sz, center_y + sz);
        glTexCoord2f(1,1);
        glVertex2i( center_x + sz, center_y + sz);
        glTexCoord2f(1,0);
        glVertex2i( center_x + sz, center_y - sz);
        
        glEnd();
        
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        ++idx;
    }
    
    glPopMatrix();
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

void RenderPassList::move(WeakPtr<RenderPass> pass, WeakPtr<RenderPass> behind_pass) {
    typedef Passes::iterator Iter;
    Iter i1 = std::find(render_passes.begin(), render_passes.end(), pass);
    if (i1 == render_passes.end()) {
        return;
    }
    Iter i2 = std::find(i1, render_passes.end(), pass);
    if (i2 != render_passes.end()) {
        // we have to move pass ( at i1) behind behind_pass ( at i2)
        render_passes.erase(i1);
        ++i2;
        render_passes.insert(i2, pass);
    }
}

