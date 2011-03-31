#ifndef IO_PANELRENDERPASS_H
#define IO_PANELRENDERPASS_H

#include <RenderPass.h>
#include "Panel.h"

namespace UI {

class PanelRenderPass : public RenderPass {
    Ptr<Panel> panel;
public:
    PanelRenderPass(JRenderer *);
    virtual ~PanelRenderPass();
    
    void setPanel(Ptr<Panel>);
    Ptr<Panel> getPanel();
    
protected:
    /// Sets render coordinate system to Eye Coordinate System and draws the panel
    /// at the resolution returned by renderer->getWidth() and renderer->getHeight()
    virtual void draw();
};

} // namespace UI

#endif

