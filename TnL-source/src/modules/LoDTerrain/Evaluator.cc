#include <interfaces/ICamera.h>
#include "LoDTerrain.h"
#include "Config.h"

LoDQuad::Evaluator::Evaluator(const Vector & p, const float plane[6][4],
        const float * vx, const float * vy, const float * vz, float focus)
: pos(p), vx(vx), vy(vy), vz(vz), focus(focus)
{
    for(int i=0; i<6; i++) for (int j=0; j<4; j++)
        this->plane[i][j] = plane[i][j];
}

float LoDQuad::Evaluator::evaluate(LoDTriangle * tri)
{
    float error=tri->error;
    float dist=calcDistance(tri->bs_center, plane[PLANE_MINUS_Z]);

    if (error < MAX_ERROR) error = MAX_ERROR;
#if USE_Z_METRIC
    error = error *
        (ERROR_FACTOR*ERROR_FACTOR) /
        (dist*dist);
#endif
    
#if USE_BOUNDING_SPHERE_METRIC
    {
        float dist2 = std::max(0.0001f, dist - tri->radius);
        error *= (ERROR_FACTOR*ERROR_FACTOR) / (dist2*dist2);
    }
#endif

#if USE_BOUNDING_SPHERE_DISTANCE_METRIC
    {
        float dist2 = (tri->bs_center-pos).length() - tri->radius;
        dist2 = std::max(0.0001f, dist2);
        //error *= (ERROR_FACTOR*ERROR_FACTOR*ERROR_FACTOR) / (dist2*dist2*dist2);
        error *= focus * (ERROR_FACTOR*ERROR_FACTOR) / (dist2*dist2);
        //error *= (ERROR_FACTOR) / (dist2);
    }
#endif

#if USE_DISTANCE_METRIC
    {
        Vector c0(vx[tri->vertex[0]], vy[tri->vertex[0]], vz[tri->vertex[0]]);
        Vector c1(vx[tri->vertex[1]], vy[tri->vertex[1]], vz[tri->vertex[1]]);
        Vector c2(vx[tri->vertex[2]], vy[tri->vertex[2]], vz[tri->vertex[2]]);
        Vector c2c0 = c0 - c2;
        Vector c2c1 = c1 - c2;
        Vector c2pos = pos - c2;
        float a = (c2c0 * c2pos) / (c2c0.lengthSquare());
        float b = (c2c1 * c2pos) / (c2c1.lengthSquare());
        float dist2;
        if (a>=0 && b>=0 && a+b<=1) { // over triangle
            dist2 = ((pos-c0) * tri->normal);
        } else {
            float d0 = (pos-c0).length();
            float d1 = (pos-c1).length();
            float d2 = (pos-c2).length();
            dist2 = std::min( std::min(d0,d1), d2 );
        }
        //float dist2 = (Vector(vx[tri->vertex[2]], vy[tri->vertex[2]], vz[tri->vertex[2]])
        //    - pos).length();
        error = error *
            (ERROR_FACTOR*ERROR_FACTOR) /
            (dist2 * dist2);
    }
#endif

#if USE_ANGULAR_METRIC
    // We take into account that the error is perceived as smaller if
    // we look onto a triangle directly from top
    Vector v0(vx[tri->vertex[0]],
            vy[tri->vertex[0]],
            vz[tri->vertex[0]]);
    float scalar_prod = pow(abs(tri->normal * (v0 - pos).normalize()), 0.01);

    error *= scalar_prod;
    //ls_error("scalar_prod: %f\n", scalar_prod);

#endif

#if USE_EDGE_METRIC
    // Lets give highly visible edges more detail
    {
        Vector v(vx[tri->vertex[2]],vy[tri->vertex[2]],vz[tri->vertex[2]]);
        v-=pos;
        if (v * tri->normal < 0) {
            for(int i=0; i<3; i++) {
                if (tri->neighbor[i]) {
                    LoDTriangle * n = tri->neighbor[i];
                    v = Vector(
                            vx[n->vertex[2]],
                            vy[n->vertex[2]],
                            vz[n->vertex[2]]);
                    v-=pos;
                    if( v * tri->neighbor[i]->normal > 0)
                    {
                        error *= 3.0;
                        tri->flags |= TFLAG_DEBUG;
                        break;
                    }
                }
            }
        }
    }
#endif // USE_EDGE_METRIC
    return error;
}

#define SAFETY_DIST 0.0

LoDQuad::Evaluator::FrustumView
LoDQuad::Evaluator::checkAgainstFrustum( const LoDTriangle * tri )
{
    float dist;
    int out_partially_flags = 0;
    /*
      We iterate through the 6 view planes, the "minus-z"-plane
      comes last. This way we can just reuse the calculated dist
      for the level of detail calculations.
    */
    for (int i=5; i >= 0; i--) {
        dist=calcDistance(tri->bs_center, plane[i]);
        if (dist < tri->radius) {
            out_partially_flags |= 2 << i;
        }
        if (dist < -tri->radius - SAFETY_DIST) return OUTSIDE;
    }

    if (out_partially_flags == 0) {
        // the sphere lies completely inside the viewing frustum
        // so we won't need any further checks for the child triangles
        return INSIDE;
    }
    return PARTIAL;
}

bool LoDQuad::Evaluator::onFrontSide(const LoDTriangle * tri) {
    int vtx = tri->vertex[2];
    return (Vector(vx[vtx], vy[vtx], vz[vtx])-pos)*tri->normal <= 0;
}

float LoDQuad::Evaluator::calcDistance(const Vector & point, const float *plane)
{
    return
        point[0]*plane[0] +
        point[1]*plane[1] +
        point[2]*plane[2] +
        plane[3];
}

