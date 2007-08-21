#ifndef IVIEW_H
#define IVIEW_H

#include <interfaces/IMovementProvider.h>

struct IActor;
class RenderPass;

/// A view tells the game how to render the world from a subjective perspective.
/// It is the combination of:
///  - An IActor, called the <strong>view subject</strong>, that this view is
///    logically (but not necessarily physically) bound to.
///  - An IMovementProvider, called the <strong>view head</strong>, marking the
///    place where the observer's head would be. This is used to control the
///    sound system's listener. Normally, the view head is identical to the
///    camera used to draw the main scene, but it can be in some completely
///    different place, too.
///  - A RenderPass giving a detailed description of how to render the scene.
///    The RenderPass may be augmented by overlaid information (game messages,
///    GUI)
/// A view can either be enabled or disabled, which is used to control
/// processes associated with a view. An actual example is the missile warning
/// module of the HUD, which uses an alarm sound that must be turned off when the
/// view is disabled.
struct IView : virtual public Object
{
    virtual Ptr<IActor> getViewSubject()=0;
    virtual Ptr<IMovementProvider> getViewHead()=0;
    virtual Ptr<RenderPass> getRenderPass()=0;
    
    /// @name Enabling and disabling.
    /// @{
    virtual void enable()=0;
    virtual void disable()=0;
    virtual bool isEnabled()=0;
    /// @}
};

#endif
