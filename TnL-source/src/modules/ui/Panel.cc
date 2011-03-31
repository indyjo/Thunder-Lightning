#include <algorithm>
#include <interfaces/IGame.h>
#include <interfaces/ICamera.h>
#include <modules/jogi/JRenderer.h>

#include "Panel.h"

namespace {
    struct HasName {
        std::string name;
        HasName(const std::string &name) : name(name) { }
        bool operator() (const std::pair<Ptr<UI::Component>, UI::Panel::LayoutInfo> & entry) const {
            return name == entry.first->getName();
        }
    };
}

namespace UI {

Panel::Panel(JRenderer *r)
:   renderer(r), enabled(false)
{
}

Panel::~Panel() { }

void Panel::addModule(
        Ptr<Component> component,
        std::string relative_to,
        int parent_corner, int child_corner,
        Vector ofs, bool ofs_in_pixels)
{
    add(component, relative_to, parent_corner, child_corner,
        ofs_in_pixels?Vector2(0):Vector2(ofs[0], ofs[1]),
        ofs_in_pixels?Vector2(ofs[0], ofs[1]):Vector2(0));
}

void Panel::add(Ptr<Component> component,
                std::string relative_to,
                int parent_corner,
                int child_corner,
                Vector2 ofs_relative,
                Vector2 ofs_absolute)
{
    LayoutInfo layout = {relative_to, parent_corner, child_corner, ofs_relative, ofs_absolute};
    components.push_back(std::make_pair(component, layout));
}

void Panel::draw(const Surface & s) {
    surface = s;
    
    if (!enabled) return;
    
    layoutComponents();
    
	typedef Components::iterator Iter;
	renderer->disableAlphaBlending();
	renderer->disableTexturing();
	renderer->setCullMode(JR_CULLMODE_NO_CULLING);
	renderer->disableZBuffer();
    
	for(Iter i=components.begin(); i!=components.end(); ++i) {
	    Ptr<Component> & component = i->first;
	    Vector topleft = surface.getOrigin() + surface.getDX()*component->getOffset()[0]
	        + surface.getDY()*component->getOffset()[1];
	    Vector bottomright = topleft + surface.getDX()*component->getWidth() +
	        + surface.getDY()*component->getHeight();
	    renderer->pushClipPlane(Vector(1,0,0), -topleft[0]);
	    renderer->pushClipPlane(Vector(-1,0,0), bottomright[0]);
	    renderer->pushClipPlane(Vector(0,-1,0), topleft[1]);
	    renderer->pushClipPlane(Vector(0,1,0), -bottomright[1]);
		component->draw(*this);
		renderer->popClipPlanes(4);
	}
	
	renderer->enableZBuffer();
}

void Panel::enable() {
    if (enabled) return;
    enabled = true;
    
	typedef Components::iterator Iter;
	for(Iter i=components.begin(); i!=components.end(); ++i) {
	    i->first->enable();
	}
}

void Panel::disable() {
    if (!enabled) return;
    enabled = false;
    
	typedef Components::iterator Iter;
	for(Iter i=components.begin(); i!=components.end(); ++i) {
	    i->first->disable();
	}
}

Ptr<ICamera> Panel::getCamera() {
    return camera;
}

void Panel::setCamera(Ptr<ICamera> c) {
    camera = c;
}


void Panel::layoutComponents() {
    typedef Components::iterator Iter;
    
    for(Iter i = components.begin(); i!= components.end(); ++i) {
        Ptr<Component> component = i->first;
        LayoutInfo layout = i->second;
        
        // give the component a chance to calculate its dimensions.
        component->onLayout(*this);
        
        Vector2 ofs = layout.ofs_absolute + Vector2(
            surface.getWidth() * layout.ofs_relative[0],
            surface.getHeight() * layout.ofs_relative[1]);
            
        Ptr<Component> parent;
        Iter j = std::find_if(components.begin(), i, HasName(layout.relative_to));
        if (j == i) {
            // name wasn't found, assume root 
            parent = new Component("root", surface.getWidth(), surface.getHeight());
        } else {
            // name _was_ found
            parent = j->first;
        }
        Vector2 parent_ofs = parent->getOffset();
    
        // First set the offset to the offset of the parent
        
        if (layout.parent_corner & LEFT) {
            ofs[0] += parent_ofs[0];
        } else if (layout.parent_corner & RIGHT) {
            ofs[0] += parent_ofs[0] + parent->getWidth();
        } else {
            ofs[0] += parent_ofs[0] + parent->getWidth()/2;
        }
        if (layout.parent_corner & TOP) {
            ofs[1] += parent_ofs[1];
        } else if (layout.parent_corner & BOTTOM) {
            ofs[1] += parent_ofs[1] + parent->getHeight();
        } else {
            ofs[1] += parent_ofs[1] + parent->getHeight()/2;
        }
        
        // Then, correct the offset to reflect the component's own anchor point
        
        if (layout.child_corner & LEFT) {
        } else if (layout.child_corner & RIGHT) {
            ofs[0] -= component->getWidth();
        } else {
            ofs[0] -= component->getWidth()/2;
        }
        if (layout.child_corner & TOP) {
        } else if (layout.child_corner & BOTTOM) {
            ofs[1] -= component->getHeight();
        } else {
            ofs[1] -= component->getHeight()/2;
        }
        
        // Finally, set the component's offset
        
        component->setOffset(ofs);
    }
}

} // namespace UI

