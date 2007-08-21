#ifndef SIMPLE_VIEW_H
#define SIMPLE_VIEW_H

#include <tnl.h>
#include <interfaces/IView.h>
#include <remap.h>

class RenderPass;

class SimpleView : virtual public IView {
public:
    SimpleView(Ptr<IActor> subject, Ptr<IMovementProvider> head, Ptr<RenderPass> pass);
    
    /// @name Configuration
    /// @{
    inline void setViewSubject(Ptr<IActor> s) { subject = s; }
    inline void setViewHead(Ptr<IMovementProvider> p) { head = p; }
    inline void setRenderPass(Ptr<RenderPass> r) { render_pass = r ; }
    /// @}
    
    
    /// Event handling. A SimpleView can be configured to catch events while
    /// enabled. Those events can be mapped to member functions of other
    /// objects. The SimpleView's address can be bound as an argument to the
    /// handler function.
    Ptr<EventSheet> getEventSheet(Ptr<EventRemapper>);
    
    /// @name IView implementation
    /// @{
    virtual Ptr<IActor> getViewSubject();
    virtual Ptr<IMovementProvider> getViewHead();
    virtual Ptr<RenderPass> getRenderPass();
    
    virtual void enable();
    virtual void disable();
    virtual bool isEnabled();
    /// @}
    
    /// @name Signals to facilitate reacting to enable()/disable()
    /// @{
    SigC::Signal0<void> & onEnable();
    SigC::Signal0<void> & onDisable();
    /// @}
    
private:
    Ptr<IActor> subject;
    Ptr<IMovementProvider> head;
    Ptr<RenderPass> render_pass;
    bool is_enabled;
    SigC::Signal0<void> on_enable, on_disable;
    Ptr<EventRemapper> event_remapper;
    Ptr<EventSheet> event_sheet;
};

#endif
