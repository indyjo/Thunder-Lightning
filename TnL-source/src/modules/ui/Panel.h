#ifndef UI_PANEL_H
#define UI_PANEL_H

#include <string>
#include <vector>

// for std::pair
#include <utility>

#include <interfaces/IDrawable.h>
#include <modules/math/Vector.h>
#include "Component.h"
#include "Surface.h"

struct ICamera;
struct IGame;
class JRenderer;

namespace UI {

class Panel : virtual public SigObject {
public:
    struct LayoutInfo {
        std::string relative_to;
        int parent_corner, child_corner;
        Vector2 ofs_relative;
        Vector2 ofs_absolute;
    };
    enum {
        LEFT=0x01,
        RIGHT=0x02,
        HCENTER=0x00,
        TOP=0x04,
        BOTTOM=0x08,
        VCENTER=0x00
    };

    Panel(JRenderer *r);
    ~Panel();
    
    /// @name Adding of components
    /// @{
    /// Adds a component to the Panel which is stacked on top of all previously
    /// added components. Its location is determined by the layout information
    /// gives as parameters.
    /// @note This is a legacy function. It is preferred to call add()
    void addModule(Ptr<Component>,
        std::string relative_to="root",
        int parent_corner=0, int child_corner=0,
        Vector ofs=Vector(0), bool ofs_in_pixels=true);
    /// Adds a component to the Panel which is stacked on top of all previously
    /// added components. Its location is determined by the layout information
    /// gives as parameters.
    /// @param component        The component to add to the Panel
    /// @param relative_to      The parent component's name
    /// @param parent_corner    The corner of the parent at which to anchor
    /// @param child_corner     The corner of the component at which to anchor
    /// @param ofs_relative     An offset to the anchor point in relative (0..1) coordinates
    /// @param ofs_absolute     An offset to the anchor point in absolute (pixel) coordinates
    void add(Ptr<Component> component,
        std::string relative_to="root",
        int parent_corner=0,
        int child_corner=0,
        Vector2 ofs_relative=Vector2(0),
        Vector2 ofs_absolute=Vector2(0));
    /// @}

    /// Queries the current surface. Only during draw() will this result in
    /// a valid surface.
    inline const UI::Surface & getSurface() { return surface; }
    /// Queries the renderer this Panel uses.
    inline JRenderer* getRenderer() { return renderer; }
    
    /// Makes the given surface current, performs layouting of components
    /// and draws them, starting from the bottom-most one.
    virtual void draw(const Surface & surface);
    
    /// @name Component enabling/disabling. Starts disabled.
    /// @{
    void enable();
    void disable();
    inline bool isEnabled() { return enabled; }
    /// @}
    
    /// @name Camera association.
    /// A Panel may be asociated to a camera for HUD-like purposes.
    /// @{
    /// If defined before, returns the associated camera. Else, returns a null
    /// pointer.
    Ptr<ICamera> getCamera();
    /// Sets the associated camera.
    void setCamera(Ptr<ICamera>);
    /// @}
    
private:
    void layoutComponents();


    Surface surface;
    JRenderer * renderer;
    Ptr<ICamera> camera;
    typedef std::vector<std::pair<Ptr<Component>, LayoutInfo> > Components;
    Components components;
    bool enabled;
};

} //namespace UI

#endif

