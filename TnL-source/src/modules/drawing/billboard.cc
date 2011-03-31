#include <cmath>
#include "billboard.h"

void drawBillboard(JRenderer* renderer,
                   Vector pos,
                   Ptr<IPositionProvider> observer,
                   float rotation,
                   float half_width,
                   float half_height,
                   float offset_frac
                  )
{
    float cos_ = cos(rotation);
    float sin_ = sin(rotation);
    
    Vector right = cos_ * observer->getRightVector() + sin_ * observer->getUpVector();
    Vector up =   -sin_ * observer->getRightVector() + cos_ * observer->getUpVector();
    
    pos = (1-offset_frac)*pos + offset_frac*observer->getLocation();
    
    renderer->begin(JR_DRAWMODE_QUADS);
    
    renderer->setUVW(Vector(0,0,0));
    renderer->vertex(pos - half_width*right - half_height*up);
    
    renderer->setUVW(Vector(0,1,0));
    renderer->vertex(pos - half_width*right + half_height*up);
    
    renderer->setUVW(Vector(1,1,0));
    renderer->vertex(pos + half_width*right + half_height*up);
    
    renderer->setUVW(Vector(1,0,0));
    renderer->vertex(pos + half_width*right - half_height*up);
    
    renderer->end();
}
