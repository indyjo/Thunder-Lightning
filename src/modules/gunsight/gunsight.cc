#include <tnl.h>
#include <interfaces/IGame.h>
#include <interfaces/ICamera.h>
#include <modules/jogi/JRenderer.h>
#include <modules/environment/environment.h>

#include "gunsight.h"

GunsightModule::GunsightModule(
	const char *name, float w, float h)
:   width(w), height(h), name(name), offset(0,0,0)
{ }

void GunsightModule::enable() { }
void GunsightModule::disable() { }

void PlaceholderModule::draw(FlexibleGunsight &) {
}


FlexibleGunsight::FlexibleGunsight(Ptr<IGame> game)
:   game(game)
{
    camera = game->getCamera();
    surface = game->getScreenSurface();
    renderer = game->getRenderer();
    env = game->getEnvironment();
    modules.push_back(new PlaceholderModule(
            "screen",
            surface.getWidth(),
            surface.getHeight()));
}

void FlexibleGunsight::addModule(
        Ptr<GunsightModule> module,
        std::string relative_to,
        int parent_corner, int child_corner,
        Vector ofs, bool ofs_in_pixels)
{
    if (!ofs_in_pixels) {
        ofs[0] *= surface.getWidth();
        ofs[1] *= surface.getHeight();
    }
    
    typedef Modules::reverse_iterator Iter;
    for(Iter i=modules.rbegin(); i!=modules.rend(); ++i) {
        if ((*i)->getName() == relative_to) {
            // copy the renderpass from the parent
            module->setRenderPass((*i)->getRenderPass());
            
            Vector parent_ofs = (*i)->getOffset();
            if (parent_corner & LEFT) {
                ofs[0] += parent_ofs[0];
            } else if (parent_corner & RIGHT) {
                ofs[0] += parent_ofs[0] + (*i)->getWidth();
            } else {
                ofs[0] += parent_ofs[0] + (*i)->getWidth()/2;
            }
            if (parent_corner & TOP) {
                ofs[1] += parent_ofs[1];
            } else if (parent_corner & BOTTOM) {
                ofs[1] += parent_ofs[1] + (*i)->getHeight();
            } else {
                ofs[1] += parent_ofs[1] + (*i)->getHeight()/2;
            }
            break;
        }
    }
    
    if (child_corner & LEFT) {
    } else if (child_corner & RIGHT) {
        ofs[0] -= module->getWidth();
    } else {
        ofs[0] -= module->getWidth()/2;
    }
    if (child_corner & TOP) {
    } else if (child_corner & BOTTOM) {
        ofs[1] -= module->getHeight();
    } else {
        ofs[1] -= module->getHeight()/2;
    }
    
    module->setOffset(ofs);
    modules.push_back(module);
}

void FlexibleGunsight::addRenderPassRoot(Ptr<RenderPass> render_pass, const char *name)
{
    render_pass->postScene().connect(
        SigC::slot(*this, &FlexibleGunsight::drawRenderPass));
    modules.push_back(new PlaceholderModule(
        name,
        render_pass->getWidth(),
        render_pass->getHeight()));
    modules.back()->setRenderPass(render_pass);
}

void FlexibleGunsight::draw() {
    drawRenderPass(0);
}

void FlexibleGunsight::drawRenderPass(Ptr<RenderPass> render_pass) {
    surface = game->getScreenSurface();
	typedef Modules::iterator Iter;
	renderer->disableAlphaBlending();
	renderer->disableTexturing();
	renderer->disableFog();
	renderer->setCullMode(JR_CULLMODE_NO_CULLING);
	renderer->setCoordSystem(JR_CS_EYE);
	renderer->disableZBuffer();
    renderer->setClipRange(0.1, 10.0);
    
	for(Iter i=modules.begin(); i!=modules.end(); ++i) {
	    Ptr<GunsightModule> & module = *i;
	    if (module->getRenderPass() != render_pass) {
	        // only draw modules for render_pass 
	        continue;
	    }
	    Vector topleft = surface.getOrigin() + surface.getDX()*module->getOffset()[0]
	        + surface.getDY()*module->getOffset()[1];
	    Vector bottomright = topleft + surface.getDX()*module->getWidth() +
	        + surface.getDY()*module->getHeight();
	    renderer->pushClipPlane(Vector(1,0,0), -topleft[0]);
	    renderer->pushClipPlane(Vector(-1,0,0), bottomright[0]);
	    renderer->pushClipPlane(Vector(0,-1,0), topleft[1]);
	    renderer->pushClipPlane(Vector(0,1,0), -bottomright[1]);
		module->draw(*this);
		renderer->popClipPlanes(4);
	}
	
    renderer->setClipRange(env->getClipMin(), env->getClipMax());
	renderer->enableZBuffer();
	renderer->setCoordSystem(JR_CS_WORLD);
	renderer->enableFog();
}

void FlexibleGunsight::enable() {
	typedef Modules::iterator Iter;
	for(Iter i=modules.begin(); i!=modules.end(); ++i) {
	    (*i)->enable();
	}
}

void FlexibleGunsight::disable() {
	typedef Modules::iterator Iter;
	for(Iter i=modules.begin(); i!=modules.end(); ++i) {
	    (*i)->disable();
	}
}


