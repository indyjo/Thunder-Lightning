#include <cmath>
#include "billboard.h"

void drawBillboard(JRenderer* renderer,
                   const Vector &pos,
                   const Vector &obs_pos,
                   const Vector &right,
                   const Vector &up,
                   float rotation,
                   float half_width,
                   float half_height,
                   float offset_frac
                  )
{
    float cos_ = cos(rotation);
    float sin_ = sin(rotation);
    
    Vector p = (1-offset_frac)*pos + offset_frac*obs_pos;
    
    renderer->begin(JR_DRAWMODE_QUADS);
    
    renderer->setUVW(Vector(0,0,0));
    renderer->vertex(p - half_width*right - half_height*up);
    
    renderer->setUVW(Vector(0,1,0));
    renderer->vertex(p - half_width*right + half_height*up);
    
    renderer->setUVW(Vector(1,1,0));
    renderer->vertex(p + half_width*right + half_height*up);
    
    renderer->setUVW(Vector(1,0,0));
    renderer->vertex(p + half_width*right - half_height*up);
    
    renderer->end();
}

void drawBillboard(JRenderer* renderer,
                   Vector pos,
                   Ptr<IPositionProvider> observer,
                   float rotation,
                   float half_width,
                   float half_height,
                   float offset_frac
                   )
{
    drawBillboard(renderer, pos, observer->getLocation(), observer->getRightVector(), observer->getUpVector(), rotation, half_width, half_height, offset_frac);
}