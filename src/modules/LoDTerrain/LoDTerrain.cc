#include "LoDTerrain.h"
#include "Config.h"
#include <algorithm>

#define LEFT 0
#define RIGHT 1
#define TOP 2
#define BOTTOM 2

typedef struct {
	char magic[4];
	ju32 triangles;
	ju32 vertices;
} LoDQuadFileHeader;



// BEGIN: LoDQuad methods

LoDQuad::LoDQuad()
{
}

LoDQuad::~LoDQuad()
{
}


void LoDQuad::init (IGame * the_game, std::istream & in, LoDQuad ** neighbor,
                    TexPtr main_tex, TexPtr detail_tex,
                    const char * texmap_name, TexPtr (*textures)[16],
                    const char * lightmap_name,
                    Status & stat)
{
    ls_message("<LoDQuad::init>\n");
    this->textures = textures;
    LoDQuadFileHeader header;
    LoDTriangleFileStruct ftriangle;
    int i;
    Vector v0, v1, v2;
    
    texmap = new Image(texmap_name);
    //texmap->saveTo("test.tga");
    
    game=the_game;
    
    this->main_tex=main_tex;
    this->detail_tex=detail_tex;
    
    in.read((char*) &header, sizeof (LoDQuadFileHeader));
    
    triangles=header.triangles;
    vertices=header.vertices;
    
    triangle = new LoDTriangle[triangles];
    //ls_warning("Init %p: (vertices=%d)\n", this, vertices);
    vx = new float[vertices];
    vy = new float[vertices];
    vz = new float[vertices];
    tex_u = new float[vertices];
    tex_v = new float[vertices];
    
    stat.beginJob("Initializing LOD quad", 7);
    
    stat.beginJob("Loading triangle data", triangles);
    for (i=0; i<triangles; i++) {
        in.read((char*) &ftriangle, sizeof(LoDTriangleFileStruct));

        triangle[i].error = ftriangle.error;
        triangle[i].flags = ftriangle.flags;
        
        if (triangle[i].flags & TFLAG_HAS_CHILDREN) {
            triangle[i].child[0] = &triangle[ftriangle.child[0]];
            triangle[i].child[1] = &triangle[ftriangle.child[1]];
        } else {
            triangle[i].child[0] = 0;
            triangle[i].child[1] = 0;
        }
        
        triangle[i].vertex[0] = ftriangle.vertex[0];
        triangle[i].vertex[1] = ftriangle.vertex[1];
        triangle[i].vertex[2] = ftriangle.vertex[2];
        
        stat.stepFinished();
    }
    stat.endJob();
    
    stat.beginJob("Loading vertex data", 3);
    in.read((char*) vx, sizeof(float) * vertices);
    stat.stepFinished();
    in.read((char*) vz, sizeof(float) * vertices);
    stat.stepFinished();
    in.read((char*) vy, sizeof(float) * vertices);
    stat.endJob();
    
    for (i=0; i<vertices; i++) {
        vz[i]=-vz[i];
        //vy[i]/=3.0;
        //vy[i]*=3.0;
        //vy[i] *= 2;
    }
    stat.stepFinished();
    
    // Measure dimensions and calc texture coords
    float x0 = vx[triangle[0].vertex[2]];
    float z0 = vz[triangle[0].vertex[2]];
    float dx = vx[triangle[0].vertex[1]] - x0;
    float dz = vz[triangle[0].vertex[0]] - z0;
    stat.beginJob("Calculating texture coordinates", vertices);
    for (i=0; i<vertices; i++) {
        tex_u[i] = (vx[i] - x0) / dx;
        tex_v[i] = (vz[i] - z0) / dz;
        stat.stepFinished();
    }
    stat.endJob();
    
    stat.beginJob("Calculating bounding spheres", 2);
    setupBoundingSpheres(&triangle[0]);
    stat.stepFinished();
    setupBoundingSpheres(&triangle[1]);
    stat.endJob();
    
    stat.beginJob("Calculating triangle normals", triangles);
    for (i=0; i<triangles; i++) {
        //triangle[i].radius = calcRadius( &triangle[i] );
        /*if (i % 1000 == 0) {
            ls_message("radius for tri #%d is %f\n", i, triangle[i].radius);
        }*/
        v0 = Vector(vx[triangle[i].vertex[0]],
                vy[triangle[i].vertex[0]],
                vz[triangle[i].vertex[0]]);
        v1 = Vector(vx[triangle[i].vertex[1]], 
                vy[triangle[i].vertex[1]], 
                vz[triangle[i].vertex[1]]);
        v2 = Vector(vx[triangle[i].vertex[2]], 
                vy[triangle[i].vertex[2]], 
                vz[triangle[i].vertex[2]]);
        triangle[i].normal= ((v2 - v0) % (v1 - v0)).normalize();
        stat.stepFinished();
    }
    stat.endJob();
    
    this->neighbor[0]=neighbor[0];
    this->neighbor[1]=neighbor[1];
    this->neighbor[2]=neighbor[2];
    this->neighbor[3]=neighbor[3];
    
    stat.beginJob("Homogenizing error value", 2);
    homogenizeError(&triangle[0]);
    stat.stepFinished();
    homogenizeError(&triangle[1]);
    stat.endJob();
    
    lightmap = game->getTexMan()->query(lightmap_name);
    
    environment = game->getEnvironment();
    
    stat.endJob();
    ls_message("</LoDQuad::init>\n");
}

void LoDQuad::done()
{
    delete[] vx;
    delete[] vy;
    delete[] vz;
    delete[] triangle;
}


void LoDQuad::connect()
{
    if (neighbor[QN_WEST]) {
        triangle[0].neighbor[0] = &neighbor[QN_WEST]->triangle[1];
    } else {
        triangle[0].neighbor[0] = 0;
    }

    if (neighbor[QN_NORTH]) {
        triangle[0].neighbor[1] = &neighbor[QN_NORTH]->triangle[1];
    } else {
        triangle[0].neighbor[1] = 0;
    }

    if (neighbor[QN_EAST]) {
        triangle[1].neighbor[0] = &neighbor[QN_EAST]->triangle[0];
    } else {
        triangle[1].neighbor[0] = 0;
    }

    if (neighbor[QN_SOUTH]) {
        triangle[1].neighbor[1] = &neighbor[QN_SOUTH]->triangle[0];
    } else {
        triangle[1].neighbor[1] = 0;
    }

    triangle[0].neighbor[2] = &triangle[1];
    triangle[1].neighbor[2] = &triangle[0];
}

void LoDQuad::presetup() {
    triangle[0].flags |= TFLAG_ENABLED;
    triangle[1].flags |= TFLAG_ENABLED;
    connect();
}

void LoDQuad::setup(Vector &pos, const float planes[6][4])
{
    evaluator = Evaluator(pos, planes, vx, vy, vz);

    triangle[0].dyn_error = evaluator.evaluate(&triangle[0]);
    triangle[1].dyn_error = evaluator.evaluate(&triangle[1]);
    
    triangle[0].morph = triangle[1].morph = 1.0;
    
    setupRecursive(&triangle[0], true);
    setupRecursive(&triangle[1], true);
}


void LoDQuad::setupBoundingSpheres(LoDTriangle * tri)
{
    if (tri->flags & TFLAG_HAS_CHILDREN) {
        setupBoundingSpheres(tri->child[0]);
        setupBoundingSpheres(tri->child[1]);
        Vector c0(tri->child[0]->bs_center);
        Vector c1(tri->child[1]->bs_center);
        Vector d = (c1 - c0).normalize();
        c0 -= tri->child[0]->radius * d;
        c1 += tri->child[1]->radius * d;
        tri->bs_center = (c0 + c1) / 2.0;
        tri->radius = (c1 - c0).length() / 2.0;
    } else {
        // TODO: compute minimal enclosing Sphere for triangle
        Vector p0(vx[tri->vertex[0]], vy[tri->vertex[0]], vz[tri->vertex[0]]);
        Vector p1(vx[tri->vertex[1]], vy[tri->vertex[1]], vz[tri->vertex[1]]);
        Vector p2(vx[tri->vertex[2]], vy[tri->vertex[2]], vz[tri->vertex[2]]);
        tri->bs_center = (p0 + p1) / 2.0;
        float d1 = (p1 - tri->bs_center).length();
        float d2 = (p2 - tri->bs_center).length();
        tri->radius = std::max( d1, d2 );
    }
}

// split2 enables a triangle's children, computes their error value and
// sets their morph factor to the maximum value
void LoDQuad::split2(LoDTriangle *tri)
{
    if (! (tri->flags & TFLAG_HAS_CHILDREN)) {
        ls_warning("Triangle doesn't have children! Aborting!\n");
        return;
    }
    
    /*ls_message("Splitting triangle %d (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)\n",
        tri_idx,
        
        );*/
    
    // calculate error and morph strength for children
    float error0, error1, morph;
    error0 = evaluator.evaluate(tri->child[0]);
    error1 = evaluator.evaluate(tri->child[1]);
    
    tri->child[0]->dyn_error = error0;
    tri->child[1]->dyn_error = error1;
    
    error0 = std::max(error0, error1);
    if (error0 > tri->dyn_error) {
        tri->child[0]->flags |= TFLAG_DEBUG;
        tri->child[1]->flags |= TFLAG_DEBUG;
        error0 = tri->dyn_error;
        tri->child[0]->dyn_error = error0;
        tri->child[1]->dyn_error = error0;
    }
    morph = 1.0 - (MAX_ERROR - error0) / (tri->dyn_error - error0);
    morph = std::min(1.0f, std::max(0.0f, morph));
    tri->child[0]->morph = tri->child[1]->morph = morph;
    
    tri->flags &= ~(TFLAG_ENABLED);
    tri->child[0]->flags &= ~(TFLAG_DONT_DRAW);
    tri->child[1]->flags &= ~(TFLAG_DONT_DRAW);
    tri->child[0]->flags |= TFLAG_ENABLED | (tri->flags & TFLAG_DONT_DRAW);
    tri->child[1]->flags |= TFLAG_ENABLED | (tri->flags & TFLAG_DONT_DRAW);
    
    tri->child[LEFT]->neighbor[BOTTOM] = tri->neighbor[LEFT];
    tri->child[RIGHT]->neighbor[BOTTOM] = tri->neighbor[RIGHT];
    
    tri->child[LEFT]->neighbor[LEFT] = tri->child[RIGHT];
    tri->child[RIGHT]->neighbor[RIGHT] = tri->child[LEFT];
    
    if (tri->neighbor[LEFT]) {
        if (tri == tri->neighbor[LEFT]->neighbor[BOTTOM] ) {
            tri->neighbor[LEFT]->neighbor[BOTTOM] = tri->child[LEFT];
        } else if (tri == tri->neighbor[LEFT]->neighbor[RIGHT] ) {
            tri->neighbor[LEFT]->neighbor[RIGHT] = tri->child[LEFT];
        } else {
            tri->neighbor[LEFT]->neighbor[LEFT] = tri->child[LEFT];
        }
    }
    
    if (tri->neighbor[RIGHT]) {
        if (tri == tri->neighbor[RIGHT]->neighbor[BOTTOM] ) {
            tri->neighbor[RIGHT]->neighbor[BOTTOM] = tri->child[RIGHT];
        } else if (tri == tri->neighbor[RIGHT]->neighbor[RIGHT] ) {
            tri->neighbor[RIGHT]->neighbor[RIGHT] = tri->child[RIGHT];
        } else {
            tri->neighbor[RIGHT]->neighbor[LEFT] = tri->child[RIGHT];
        }
    }
}

void LoDQuad::split(LoDTriangle *tri)
{
    // Don't split triangles that have already been split
    if ((tri->flags & TFLAG_ENABLED) == 0) {
        ls_error("Trying to split triangle@%p which has already been split!\n",
                tri);
        return;
    }
    
    if (tri->neighbor[BOTTOM]) {
        if (tri != tri->neighbor[BOTTOM]->neighbor[BOTTOM]) {
            split(tri->neighbor[BOTTOM]);
        }
        // Now we have a diamond configuration:
        //    /\
        //   /__\  <- our triangle tri
        //   \  /  <- tri->neighbor[BOTTOM]
        //    \/
        split2(tri);
        split2(tri->neighbor[BOTTOM]);
        
        // Make sure the diamond has a uniform morph value in all four
        // triangles. Since split2 gives the two children of a triangle already
        // a uniform morph value, we just have to find which triangle's
        // children have the bigger value and then copy it to the other
        // triangle's children.
        if (tri->child[0]->morph > tri->neighbor[BOTTOM]->child[0]->morph) {
            tri->neighbor[BOTTOM]->child[0]->morph =
                    tri->neighbor[BOTTOM]->child[1]->morph =
                    tri->child[0]->morph;
        } else {
            tri->child[0]->morph = tri->child[1]->morph =
                    tri->neighbor[BOTTOM]->child[0]->morph;
        }
        
        tri->child[LEFT]->neighbor[RIGHT]=
            tri->neighbor[BOTTOM]->child[RIGHT];
        tri->child[RIGHT]->neighbor[LEFT]=
            tri->neighbor[BOTTOM]->child[LEFT];
        tri->neighbor[BOTTOM]->child[LEFT]->neighbor[RIGHT]=
            tri->child[RIGHT];
        tri->neighbor[BOTTOM]->child[RIGHT]->neighbor[LEFT]=
            tri->child[LEFT];
    } else {
        split2(tri);
        tri->child[LEFT]->neighbor[RIGHT]=0;
        tri->child[RIGHT]->neighbor[LEFT]=0;
    }
}



#define OUTMASK_MINUS_X  ( 2 << PLANE_MINUS_X )
#define OUTMASK_PLUS_X   ( 2 << PLANE_PLUS_X  )
#define OUTMASK_MINUS_Y  ( 2 << PLANE_MINUS_Y )
#define OUTMASK_PLUS_Y   ( 2 << PLANE_PLUS_Y  )
#define OUTMASK_MINUS_Z  ( 2 << PLANE_MINUS_Z )
#define OUTMASK_PLUS_Z   ( 2 << PLANE_PLUS_Z  )


void LoDQuad::setupRecursive (LoDTriangle *tri, bool partially_obscured)
{
    float error;
    Evaluator::FrustumView frustum_view;
    
    // We clear all flags that might still be set from the previous setup
    tri->flags &= ~(TFLAG_DETAIL_TEX | TFLAG_DEBUG | TFLAG_DONT_DRAW);
    
    if ((tri->flags & TFLAG_ENABLED) == 0) {
        setupRecursive(tri->child[0], true);
        setupRecursive(tri->child[1], true);
        
        // if both child triangles are invisible then also dont draw this one
        tri->flags |= TFLAG_DONT_DRAW
                & tri->child[0]->flags & tri->child[1]->flags;
        
    } else {
        if (partially_obscured) {
            frustum_view = evaluator.checkAgainstFrustum(tri);
            partially_obscured = (frustum_view == Evaluator::PARTIAL);
            if (frustum_view == Evaluator::OUTSIDE) {
                tri->flags |= TFLAG_DONT_DRAW | TFLAG_ENABLED;
                return;
            }
        } else {
            frustum_view = Evaluator::INSIDE;
        }
        
        // Now frustum_view is either INSIDE or PARTIAL
        
        error = tri->dyn_error;

        if ((error > MAX_ERROR) && (tri->flags & TFLAG_HAS_CHILDREN))
        {
            split(tri);
            setupRecursive(tri->child[0], partially_obscured);
            setupRecursive(tri->child[1], partially_obscured);

            // if both child triangles are invisible then also dont draw
            // this one
            tri->flags |= TFLAG_DONT_DRAW
                    & tri->child[0]->flags & tri->child[1]->flags;
        } else {
            // Test wether the triangle's front side is visible
            //if (!evaluator.onFrontSide(tri))
            //    tri->flags |= TFLAG_DONT_DRAW;
            tri->flags |= TFLAG_ENABLED;
        }
    }
}



#define HOMOGENIZE_DELTA 0.05f

float LoDQuad::homogenizeError(LoDTriangle * tri) {
    if (tri->flags & TFLAG_HAS_CHILDREN) {
        float max_child_error = std::max(
                homogenizeError(tri->child[0]),
                homogenizeError(tri->child[1]));
        tri->error = std::max( tri->error, HOMOGENIZE_DELTA + max_child_error);
        return tri->error;
    } else return tri->error;
}

