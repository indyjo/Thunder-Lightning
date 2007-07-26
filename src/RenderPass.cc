#include <algorithm>
#include <stack>

#include <GL/gl.h>

#include <modules/math/MatrixVector.h>
#include <modules/jogi/JRenderer.h>
#include <debug.h>
#include "RenderPass.h"

RenderPass::RenderPass(JRenderer *r)
    : renderer(r)
    , enabled(true)
    , clear_depth_enabled(true)
    , clear_color_enabled(false)
    , background_color(Vector(0,0,0))
{
}

RenderPass::~RenderPass()
{ }

void RenderPass::setEnabled(bool b) { enabled = b; }
bool RenderPass::isEnabled() { return enabled; }
    
Ptr<Texture> RenderPass::dependsOn(Ptr<RenderPass> other, int width, int height) {
    jrtxtid_t txtid;
    renderer->createEmptyTexture(JR_FORMAT_RGB, width, height, &txtid);
    Ptr<Texture> tex = new Texture(txtid, *renderer);
    renderer->setWrapMode(JR_TEXDIM_U, JR_WRAPMODE_CLAMP);
    renderer->setWrapMode(JR_TEXDIM_V, JR_WRAPMODE_CLAMP);
    
    dependencies.push_back(std::make_pair(other, tex));
    return tex;
}

void RenderPass::addDependency(Ptr<RenderPass> other, Ptr<Texture> tex) {
    dependencies.push_back(std::make_pair(other, tex));
}

void RenderPass::stackedOn(Ptr<RenderPass> other) {
    stack_parent = other;
}

Ptr<RenderPass> RenderPass::getBottomOfStack() {
    Ptr<RenderPass> cursor = this;
    while(cursor->stack_parent) {
        cursor = cursor->stack_parent;
    }
    return cursor;
}
    
void RenderPass::setBackgroundColor(const Vector& c) { background_color = c; }
const Vector& RenderPass::getBackgroundColor() { return background_color; }
    
void RenderPass::enableClearDepth(bool b) { clear_depth_enabled = b; }
bool RenderPass::isClearDepthEnabled() { return clear_depth_enabled; }
    
void RenderPass::enableClearColor(bool b) { clear_color_enabled = b; }
bool RenderPass::isClearColorEnabled() { return clear_color_enabled; }
    
RenderPass::RenderSignal & RenderPass::preDraw() { return pre_draw; }
RenderPass::RenderSignal & RenderPass::postDraw() { return post_draw; }
RenderPass::RenderSignal & RenderPass::preDepends() { return pre_depends; }
    
void RenderPass::render() {
    preDepends().emit(this);
    
    if (enabled) {
        for(Dependencies::iterator i = dependencies.begin(); i!= dependencies.end(); ++i) {
            i->first->renderToTexture(i->second);
        }
    }
    
    if(stack_parent) {
        stack_parent->render();
    }
    
    if (clear_color_enabled) {
        jcolor3_t col;
        col.r = 255*background_color[0];
        col.g = 255*background_color[1];
        col.b = 255*background_color[2];
        renderer->setBackgroundColor(&col);
    }
    renderer->clear(clear_color_enabled, clear_depth_enabled);
    
    if (enabled) {
        preDraw().emit(this);
        draw();
        postDraw().emit(this);
    }
}

void RenderPass::renderToTexture(Ptr<Texture> tex) {
    int saved_width = renderer->getWidth();
    int saved_height = renderer->getHeight();
    renderer->resize(tex->getWidth(), tex->getHeight());
    
    render();
    
    glReadBuffer(GL_BACK);
    renderer->setTexture(tex->getTxtid());
    glCopyTexImage2D(GL_TEXTURE_2D, 0,
        GL_RGB,
        0,0,tex->getWidth(),tex->getHeight(),
        0);
        
    renderer->resize(saved_width, saved_height);
}

void RenderPass::draw() {

}

void RenderPass::drawMosaic() {
    typedef std::vector<Ptr<Texture> > Textures;
    Textures textures;
    
    std::stack<Ptr<RenderPass> > passes;
    passes.push(this);
    while ( ! passes.empty() ) {
        Ptr<RenderPass> pass = passes.top();
        ls_message("Popping pass %p\n", ptr(pass));
        passes.pop();
        
        if (pass->stack_parent) {
            passes.push(pass->stack_parent);
            ls_message("Pushing stack parent %p\n", ptr(pass->stack_parent));
        }
        
        for(Dependencies::iterator i = pass->dependencies.begin(); i!= pass->dependencies.end(); ++i) {
            textures.push_back(i->second);
            passes.push(i->first);
            ls_message("Pushing dependency %p\n", ptr(i->first));
        }
    }
    
    int ntex = textures.size();
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
    
    int idx = 0;
    for( Textures::iterator i=textures.begin(); i!= textures.end(); ++i) {
        int r = idx / cols;
        int c = idx % cols;
        int center_x = (2*c+1) * vp_width / cols / 2;
        int center_y = (2*r+1) * vp_height / rows / 2;

        //ls_message("Mosaic tile. r=%d c=%d center_x=%d center_y=%d sz=%d\n",
        //    r,c,center_x,center_y,sz);
        
        GLuint tex = (*i)->getGLTex();
        
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

