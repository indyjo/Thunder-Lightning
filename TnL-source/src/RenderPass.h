#ifndef TNL_RENDERPASS_H
#define TNL_RENDERPASS_H

#include <list>
#include <modules/texman/TextureManager.h>
#include <modules/math/Vector.h>
#include <Weak.h>

class RenderPassInstance;
class RenderPassList;
class JRenderer;

/// This class abstracts a render pass. A render pass consists of
/// optionally cleaning the depth and color buffers and drawing something.
/// Information about output width/height and whether to render into a texture
/// or not is _not_ stored for the render pass itself but for its dependencies.

/// A render pass might be stacked on another pass, in which case stackedOn()
/// must be called before drawing. This is the usual way of compositing, e.g. a
/// background and a foreground image.

class RenderPass : public SigObject, public Weak
{
public:
    RenderPass(JRenderer *r);
    ~RenderPass();
    
    JRenderer * getRenderer();
    
    /// Sets whether this RenderPass will be drawn (true) or not (false).
    /// Default to true.
    /// @note this will also enable or disable depenent render passes, but not
    ///       the render pass this one is stacked upon.
    void setEnabled(bool);
    bool isEnabled();
    
    /// @name Connections to other RenderPass instances.
    /// @{
    
    /// Registers a render-to-texture dependency which will be executed every
    /// time before this render pass is executed.
    /// @note Must be called before render() is called.
    /// @note Normally, textures must be square, i.e. width == height.
    /// @note It is recommended to choose texture sizes which are a power of 2.
    /// @param width  the x size of the texture
    /// @param height the y size of the texture
    /// @return a texture that will contain the render passes result image
    Ptr<Texture> dependsOn(Ptr<RenderPass> other, int width, int height);
    
    /// Like dependsOn() registers a render-to-texture dependency but doesn't
    /// create a new texture. Instead, it will recycle the given texture.
    void addDependency(Ptr<RenderPass> other, Ptr<Texture> tex);
    
    /// Removes a dependency identified by the texture it renders to
    void removeDependency(Ptr<Texture> tex);
    /// Removes a dependency identified by its RenderPass
    void removeDependency(Ptr<RenderPass> pass);
    
    /// Removes all registered dependencies
    void clearDependencies();
    
    /// Configures this render pass to be stacked on top of another render pass.
    /// @param other The parent in the render pass stack. May be set to 0
    ///              to clear the stacked-on relationship.
    void stackedOn(Ptr<RenderPass> other);
    
    /// Traverses the stacked-on list and returns the bottom-most element.
    Ptr<RenderPass> getBottomOfStack();
    
    /// @}
    
    /// @name Clearing of depth and color buffer before rendering
    /// @{

    /// Sets background color used when clearing color buffer. Defaults to black.
    void setBackgroundColor(const Vector&);
    const Vector &getBackgroundColor();
    
    /// Enables depth buffer clearing. Defaults to true
    void enableClearDepth(bool);
    bool isClearDepthEnabled();
    
    /// Enables color buffer clearing. Defaults to false
    void enableClearColor(bool);
    bool isClearColorEnabled();
    
    /// @}
    
    /// @name Signals to perform customized drawing
    /// @{
    typedef SigC::Signal1<void, Ptr<RenderPass> > RenderSignal;
    /// This signal is fired before dependencies are rendered
    RenderSignal & preDepends();
    /// This signal is fired before draw() gets executed
    RenderSignal & preDraw();
    /// This signal is fired after draw() gets executed
    RenderSignal & postDraw();
    /// @}
    
    /// Triggers a render to the currently active frame buffer.
    /// This will perform actions in the following order:
    ///   # Execute the render to texture passes of all dependencies in the stack,
    ///   # draw the stack parents,
    ///   # optionally clear color and/or depth buffer,
    ///   # emit the preDraw() signal,
    ///   # call draw() and,
    ///   # emit the postDraw() signal.
    /// @note if this render pass is disabled, its dependencies are not rendered,
    ///       neither color nor depth buffer are cleared,
    ///       draw is not called and no signals are emitted.
    void render();
    
    /// Triggers a render to a texture.
    /// The result of calling this after render() is undefined as rendering to
    /// a texture may overwrite data in the main render buffer.
    /// Otherwise, this function will perform the same sequence of operations
    /// as render().
    void renderToTexture(Ptr<Texture>);
    
    /// Debugging function that draws a mosaic of all generated textures.
    void drawMosaic();
    
protected:
    /// Do the drawing associated with this render pass. This function may
    /// be overridden by child classes. The default does nothing.
    virtual void draw();
    
private:
    JRenderer *renderer;
    
    Ptr<RenderPass> stack_parent;
    
    typedef std::pair<Ptr<RenderPass>, Ptr<Texture> > Dependency;
    typedef std::vector<Dependency> Dependencies;
    Dependencies dependencies;

    bool enabled, clear_depth_enabled, clear_color_enabled;
    Vector background_color;

    RenderSignal pre_draw, post_draw, pre_depends;
};


#endif

