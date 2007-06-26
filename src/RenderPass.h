#ifndef TNL_RENDERPASS_H
#define TNL_RENDERPASS_H

#include <list>
#include <GL/gl.h>
#include <modules/math/Vector.h>
#include <RenderContext.h>
#include <Weak.h>

struct IGame;
class RenderPassList;
class JRenderer;

/// This class abstracts a full render pass. A render pass begins with setting
/// up the render target (width, height, render to texture, cleaning), optionally
/// drawing a scene defined by a RenderContext and drawing customized graphics
/// before or after that scene. A RenderPass belongs to a RenderPassList which
/// is responsible for creation of render passes and managing the order in which
/// these are performed. A render pass gets performed as long as
///   - it is enabled
///   - it hasn't been deleted
///   - the RenderPassList hasn't been deleted
/// The RenderPassList maintains a weak reference to the RenderPass, so it
/// doesn't keep it alive. Conversely, the RenderPass keeps a weak reference
/// to the RenderPassList.
class RenderPass : public Object, public Weak
{
    friend class RenderPassList;
    
    RenderPass(WeakPtr<RenderPassList>);
    
public:
    ~RenderPass();
    
    /// Sets whether this RenderPass will be drawn (true) or not (false).
    /// Default to true.
    void setEnabled(bool);
    bool isEnabled();
    
    /// If other comes after this render pass in the RenderPassList,
    /// move this pass to the place right behind other.
    /// The default place for a newly created RenderPass is at the
    /// beginning of the list.
    void dependsOn(Ptr<RenderPass> other);
    
    /// Sets resolution of render. Defaults to 256x256.
    void setResolution(int resx, int resy);
    int getWidth();
    int getHeight();
    
    /// Sets background color used when clearing color buffer. Defaults to black.
    void setBackgroundColor(const Vector&);
    const Vector &getBackgroundColor();
    
    /// Enables depth buffer clearing. Defaults to true
    void enableClearDepth(bool);
    bool isClearDepthEnabled();
    
    /// Enables color buffer clearing. Defaults to false
    void enableClearColor(bool);
    bool isClearColorEnabled();
    
    /// Rendering can target a texture
    void setRenderToTexture(bool);
    bool isRenderToTexture();
    GLuint getTexture();
    
    /// The render context determines how the main scene gets drawn.
    void setRenderContext(const RenderContext &ctx);
    RenderContext &getRenderContext();
    void setRenderContextEnabled(bool);
    bool isRenderContextEnabled();
    
    typedef SigC::Signal1<void, Ptr<RenderPass> > RenderSignal;
    /// This signal is fired before the main scene gets drawn
    RenderSignal & preScene();
    /// This signal is fired after the main scene was draw
    RenderSignal & postScene();
    
private:
    /// Called by RenderPassList
    void beginRender(JRenderer *);
    /// Called by RenderPassList
    void endRender(JRenderer *);
    
    void createTex();
    void destroyTex();

    WeakPtr<RenderPassList> renderpasslist;
    bool enabled, context_enabled, rendertotex_enabled,
        clear_depth_enabled, clear_color_enabled;
    int width, height;
    bool tex_created, tex_needs_update;
    GLuint tex;
    RenderContext context;
    Vector background_color;
    RenderSignal pre_scene, post_scene;
};

class RenderPassList: public Object, public Weak {
    friend class RenderPass;
public:

    RenderPassList(WeakPtr<IGame>);
    ~RenderPassList();
    
    /// Creates a new RenderPass, disabled and at the front of the list
    Ptr<RenderPass> createRenderPass();
    
    /// Do the actual rendering of the passes
    void renderPasses();
    
    /// Debug function to draw a mosaic of all textures on the main surface
    void drawMosaic();

private:
    /// Called by the individual render passes to implement dependencies
    void move(WeakPtr<RenderPass> pass, WeakPtr<RenderPass> behind_pass);
    
    typedef std::list<WeakPtr<RenderPass> > Passes;
    Passes render_passes;
    WeakPtr<IGame> thegame;
};


#endif

