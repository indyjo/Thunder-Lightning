#include "camera.h"
#include <modules/math/SpecialMatrices.h>
#include <modules/environment/environment.h>
#include <interfaces/IConfig.h>
#include <remap.h>

Camera::Camera(IGame *thegame)
{
    Ptr<IConfig> config=thegame->getConfig();
    setFocus(config->queryFloat("Camera_focus",1.5));
    setAspect(config->queryFloat("Camera_aspect",1.5));
    setNearDistance(thegame->getEnvironment()->getClipMin());
    setFarDistance(thegame->getEnvironment()->getClipMax());
    
    Ptr<EventRemapper> remap = thegame->getEventRemapper();
    remap->map("zoom-in", SigC::slot(*this, &Camera::zoomIn));
    remap->map("zoom-out", SigC::slot(*this, &Camera::zoomOut));
}

static const float zoom_factor = pow(2.0f, 1.0f/6);

void Camera::zoomIn() {
    setFocus(getFocus() * zoom_factor);
}

void Camera::zoomOut() {
    setFocus(getFocus() / zoom_factor);
}

