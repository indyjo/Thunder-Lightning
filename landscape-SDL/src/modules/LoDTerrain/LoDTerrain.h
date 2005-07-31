#ifndef _LODTERRAIN_H
#define _LODTERRAIN_H

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <unistd.h>

#include <landscape.h>
#include <interfaces/ILoDQuadManager.h>
#include <interfaces/IGame.h>

#include <modules/jogi/jogi.h>
#include <modules/texman/TextureManager.h>

#include <modules/environment/environment.h>
#include <modules/math/Vector.h>
#include <modules/ui/status.h>
#include "image.h"

/*
Triangle references contain a triangle's index in the triangle list of a quad.
The 3 most significant bits of such an index determine whether the triangle
is a member of the current quad's triangle list or one of the quads neighbors.
*/
#define TREF_MASK  0xe0000000
#define TREF_INDEX 0x1fffffff
#define TREF_THIS  (5 << 29)
#define TREF_NORTH (1 << 29)
#define TREF_SOUTH (2 << 29)
#define TREF_WEST  (3 << 29)
#define TREF_EAST  (4 << 29)

/*
Triangle flags control the behavior of triangles.
First, those flags, which are rather static, are loaded from disk
*/
#define TFLAG_HAS_CHILDREN        (1 <<  0)
#define TFLAG_ALWAYS_SUBDIVIDE    (1 <<  1)

/*
Then there are these flags which are set at runtime
TFLAG_ENABLED is set by the quad's setup function, depending on whether the
triangle is to be shown on screen.
TFLAG_DETAIL_TEX, when set, signals the triangle drawin function to overlay
the triangle with the detail texture.
TFLAG_DEBUG tells the triangle renderer to draw this triangle in a special
color
*/
#define TFLAG_ENABLED             (1 << 31)
#define TFLAG_DETAIL_TEX          (1 << 30)
#define TFLAG_DEBUG               (1 << 29)
#define TFLAG_DONT_DRAW           (1 << 28)

/*
The index symbols of the quad neighbors
*/
#define QN_NORTH 0
#define QN_SOUTH 1
#define QN_EAST  2
#define QN_WEST  3

class LoDQuad;
class LoDQuadManager;

/*
The Triangle structs describe the triangles'
representation on disk and in memory respectively.
*/
typedef struct {
    ju32 child[2];    /* The indices of the triangles children (if any) */
    ju32 neighbor[3]; /* TREF to the triangles neighbors                */
    ju32 vertex[3];   /* The indices of the triangles corner vertices   */
    float error;      /* The geometry error of this triangle            */
    ju32 flags;       /* The triangle's flags                           */
} LoDTriangleFileStruct;

typedef struct _LoDTriangle {
    struct _LoDTriangle *child[2];    /* The triangles children  */
    struct _LoDTriangle *neighbor[3]; /* The triangles neighbors */
    ju32 vertex[3];    /* The indices of the triangle's vertices */
    float error;       /* The geometry error of this triangle    */
    ju32 flags;        /* The triangle's flags                   */
    Vector bs_center;  /* The bounding sphere's center           */
    float radius;      /* The radius of a sphere that contains
                          the triangle. The sphere's origin is
                          bs_center                               */
    Vector normal;     /* The triangle's normal, pointing upwards */
    float morph;       /* [0..1] 1 if triangle has full shape     */
    float dyn_error;   /* Error of the triangle on the screen     */
} LoDTriangle;

typedef struct {
    bool left:1, right:1, bottom:1;
} BorderSet;

class LoDQuad
{
    friend class Evaluator;
    class Evaluator {
        Vector pos;
        float plane[6][4];
        const float *vx, *vy, *vz;
    public:
        enum FrustumView {INSIDE, OUTSIDE, PARTIAL};
    
        inline Evaluator() { }
        Evaluator(const Vector & p, const float plane[6][4],
            const float * vx, const float * vy, const float * vz);
        float evaluate(LoDTriangle * tri);
        FrustumView checkAgainstFrustum(const LoDTriangle * tri);
        bool onFrontSide(const LoDTriangle * tri);
    private:
        static float calcDistance(const Vector & point, const float *plane);
    };
    
    friend class LoDQuadManager;
    
public:
    typedef enum {ABOVE, INSIDE, BELOW} CoordRel;
    LoDQuad();
    ~LoDQuad();
    
    void init(IGame         * the_game,
              std::istream  & in,
              LoDQuad      ** neighbor,
              TexPtr          main_tex,
              TexPtr          detail_tex,
              const char    * texmap_name,
              TexPtr        (*textures)[16],
              const char    * lightmap_name,
              Status        & stat);
    void done();
    
    void connect();
    
    void presetup();
    void setup(Vector &pos, const float planes[6][4]);
    void draw(JRenderer *renderer);
    void drawWire(JRenderer *renderer);
    CoordRel getCoordRelX(float x);
    CoordRel getCoordRelZ(float z);
    float getHeightAt(float x, float z);
    bool getHeightAtTriangle(LoDTriangle *tri, float x, float z, float *height);
    
private:
    void drawRecursive(JRenderer *r, LoDTriangle *tri,
        float y0, float y1, float y2, float *alpha);
    void drawWireBorder(JRenderer *r, LoDTriangle *tri, int i0, int i1);
    void drawWireRecursive(JRenderer *r, LoDTriangle *tri, BorderSet);

    void setupBoundingSpheres(LoDTriangle *tri);
    
    void split2(LoDTriangle *tri);
    void split(LoDTriangle *tri);
    
    void setupRecursive(LoDTriangle *tri, bool partially_obscured);
    
    float homogenizeError(LoDTriangle * tri);
    bool lineCollides(Vector a, Vector b, float * t, LoDTriangle * tri);
    void drawTexturedTriangle(JRenderer *r, LoDTriangle * tri, float *y);
    void drawFogTriangle(JRenderer *r, LoDTriangle * tri, float *y, float *alpha);

private:
    IGame *game;
    LoDQuad *neighbor[4];
    LoDTriangle *triangle;
    float *vx, *vy, *vz;
    float *tex_u, *tex_v;
    int triangles, vertices;
    TexPtr main_tex;
    TexPtr detail_tex;
    Evaluator evaluator;
    TexPtr (*textures)[16];
    Ptr<Image> texmap;
    TexPtr lightmap;
    Ptr<Environment> environment;
};

class LoDQuadManager: public ILoDQuadManager, virtual public SigObject
{
public:
    LoDQuadManager(IGame *the_game, Status &);
    virtual ~LoDQuadManager();
    
    void toggleDebugMode();
        
    virtual void draw();
    
    virtual float getHeightAt(float x, float z);
    virtual bool lineCollides(Vector a, Vector b, Vector * x);
    
private:
    LoDQuad *getQuadAtPoint(float x, float z);
    void loadTextures();

private:
    IGame *game;
    JRenderer *renderer;
    Ptr<IConfig> cfg;
    Ptr<ICamera> camera;
    LoDQuad *quad;
    int width, height;
    int counter;
    bool debug_mode;
    TexPtr textures[256][16];
};

#endif
