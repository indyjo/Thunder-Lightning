#ifndef UI_SURFACE_H
#define UI_SURFACE_H

#include <modules/math/MatrixVector.h>


namespace UI {

/** Defines a 2D drawing surface in 3D space.
  * A surface has an origin, and two directional vectors: dx and dy.
  * The surface does not imply orientation, i.e. whether dy points upwards
  * or downwards. By convention, dx points to the right and dy downwards.
  * Every UI class using Surface adheres to this convention.
  */
class Surface {
    /// The surface origin
    Vector origin;
    /// The surface's +x and +y vectors
    Vector dx, dy;

public:
    /** Creates a Surface from the given values. */
    inline Surface(const Vector & origin, const Vector & dx, const Vector & dy)
    :   origin(origin), dx(dx), dy(dy)
    { }

    /** Creates a Surface from three corners plus width and height.
      * The corners are: northwest, northeast, southwest. dx will point
      * to the east, dy to the south. The origin will be the northwest corner.
      * @param nw       the northwest corner
      * @param ne       the northeast corner
      * @param sw       the southwest corner
      * @param width    the width of the surface: nw+width*dx == ne
      * @param height   the height of the surface: nw+height*dy == sw
      * @return the calculated surface
      */
    inline static Surface FromCorners(  const Vector & nw,
                                        const Vector & ne,
                                        const Vector & sw,
                                        float width=1, float height=1)
    {
        return Surface(nw, (ne-nw) / width, (sw-nw) / height);
    }

    /** Creates a Surface from the given camera specification.
      * The surface is constructed relative to the camera.
      * @param aspect   the ratio of width/height of the camera
      * @param focus    the camera focus distance
      * @param width    the desired width of the surface
      * @param height   the desired height of the surface
      * @param factor   A distance factor. A value of 2 doubles the distance of
      *                 the surface to the camera.
      * @return a surface where the origin is the upper left corner, dx points to
      *         the right and dy to the bottom.
      */
    inline static Surface FromCamera( float aspect,
                                      float focus,
                                      float width=1,
                                      float height=1,
                                      float factor=1)
    {
        float dx2 = aspect * factor;
        float dy2 = factor;
        float dist = focus * factor;
        float f = 2/factor;
        return Surface(Vector(-dx2, dy2, dist),
                       Vector(f*dx2/width, 0, 0),
                       Vector(0, -f*dy2/height, 0));
    }

    /// Translate the origin by the specified surface units
    inline void translateOrigin(float x, float y) {
        origin += x*dx;
        origin += y*dy;
    }

    inline const Vector & getOrigin() const { return origin; }
    inline void setOrigin(const Vector & origin) { this->origin = origin; }

    inline const Vector & getDX() const { return dx; }
    inline void setDX(const Vector & dx) { this->dx = dx; }

    inline const Vector & getDY() const { return dy; }
    inline void setDY(const Vector & dy) { this->dy = dy; }

    /// Creates a 4x4 matrix M so that M*x == origin + x[0]*dx + x[1]*dy
    inline Matrix getMatrix() const {
        return Matrix(dx[0], dy[0], 0, origin[0],
                      dx[1], dy[1], 0, origin[1],
                      dx[2], dy[2], 0, origin[2],
                          0,     0, 0,         1);
    }
};


} // namespace UI

#endif

