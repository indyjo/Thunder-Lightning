#ifndef TNL_BILLBOARD_H
#define TNL_BILLBOARD_H

#include <interfaces/IPositionProvider.h>
#include <modules/jogi/JRenderer.h>
#include <tnl.h>

/// Draws the currently selected texture as a billboard sprite.
/// It will draw in a way that
///  - the sprite is facing the camera, i.e. its four corners lie in a plane
///    that is parallel to the observer's XY plane.
///  - it is oriented so that its right vector is equal to the observer's X vector
///    rotated counter-clockwise by the given angle.
/// The function will use the current rendering settings of the renderer.
/// By default, the billboard will not be drawn at the exact location given, but
/// at a point slightly offset towards the observer. The fraction to offset is given
/// in offset_frac.
void drawBillboard(JRenderer* renderer,             //< The renderer to use
                   const Vector &pos,               //< The 3d position at which to draw
                   const Vector &obs_pos,           //< The observer's position
                   const Vector &right,             //< The observer's right vector
                   const Vector &up,                //< The observer's up vector
                   float rotation=0,                //< The angle in radians by which to rotate the sprite in CCW sense
                   float half_width=1,              //< The sprite's half width
                   float half_height=1,             //< The sprite's half height
                   float offset_frac=0.1            //< Fraction of the dist between observer and pos to offset towards observer
);

/// Overloaded version that takes an IPositionReceiver argument.
void drawBillboard(JRenderer* renderer,             //< The renderer to use
                   Vector pos,                      //< The 3d position at which to draw
                   Ptr<IPositionProvider> observer, //< The observer to align with
                   float rotation=0,                //< The angle in radians by which to rotate the sprite in CCW sense
                   float half_width=1,              //< The sprite's half width
                   float half_height=1,             //< The sprite's half height
                   float offset_frac=0.1            //< Fraction of the dist between observer and pos to offset towards observer
                  );

#endif

