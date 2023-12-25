#include "Surface.h"
#include "PanelRenderPass.h"

namespace UI {

PanelRenderPass::PanelRenderPass(JRenderer *r) : RenderPass(r) {
    if (!r) {
        throw std::invalid_argument("PanelRenderPass: r is null");
    }
}
PanelRenderPass::~PanelRenderPass() { }
    
void PanelRenderPass::setPanel(Ptr<Panel> p) {
    if (panel && p != panel) panel->disable();
    panel = p;
    if (panel) panel->enable();
}
Ptr<Panel> PanelRenderPass::getPanel() { return panel; }
    
void PanelRenderPass::draw() {
    if (!panel) {
        return;
    }
    
    JRenderer * renderer = getRenderer();
    
    Surface surface = Surface::FromCamera(
        renderer->getAspect(),
        renderer->getFocus(),
        renderer->getWidth(),
        renderer->getHeight(),
        (renderer->getClipNear()+renderer->getClipFar())/2);
    
	renderer->setCoordSystem(JR_CS_EYE);
	renderer->disableFog();
	panel->draw(surface);
	renderer->enableFog();
	renderer->setCoordSystem(JR_CS_WORLD);
}

} // namespace UI

