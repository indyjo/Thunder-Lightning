#include "LoDTerrain.h"

// BEGIN: LoDQuadManager methods

LoDQuadManager::LoDQuadManager(IGame *the_game, Status & stat)
{
    int        u,v;
    char       buf[256];
    std::ifstream   in;
    LoDQuad*   (neighbor[4]);
    TexPtr     main_tex;
    TexPtr     detail_tex;
    JSprite    spr;

    std::string terrain_prefix;
    std::string texture_prefix;
    std::string texmap_prefix;
    std::string lightmap_prefix;
    int quads_w;
    int quads_h;
    std::string detail_tex_name;

    game=the_game;
    cfg = Ptr<IConfig>();
    cfg    = game->getConfig();
    camera = game->getCamera();
    renderer = game->getRenderer();

    // Get the terrain and texture paths from the config
    terrain_prefix=cfg->query("LoDQuadManager_terrain_prefix");
    texture_prefix=cfg->query("LoDQuadManager_texture_prefix");
    detail_tex_name=cfg->query("LoDQuadManager_detail_tex_name");
    texmap_prefix=cfg->query("LoDQuadManager_texmap_prefix");
    lightmap_prefix=cfg->query("LoDQuadManager_lightmap_prefix");

    // Get the texture grid size from the config
    quads_w=atoi(cfg->query("LoDQuadManager_quads_w"));
    quads_h=atoi(cfg->query("LoDQuadManager_quads_h"));

    // First load the detail texture
    detail_tex = game->getTexMan()->query(
            detail_tex_name.c_str(), JR_HINT_GREYSCALE, 0, true);

    width=quads_w;
    height=quads_h;
    quad=new LoDQuad[width*height];

    stat.beginJob("Loading quads", width*height);
    for (v=0; v<height; v++) {
        for (u=0; u<width; u++) {
            if (v>0) {
                neighbor[QN_NORTH]=&quad[(v-1)*width + u];
            } else {
                neighbor[QN_NORTH]=0;
            }

            if (v<(height-1)) {
                neighbor[QN_SOUTH]=&quad[(v+1)*width + u];
            } else {
                neighbor[QN_SOUTH]=0;
            }

            if (u>0) {
                neighbor[QN_WEST]=&quad[v*width + u-1];
            } else {
                neighbor[QN_WEST]=0;
            }

            if (u<(width-1)) {
                neighbor[QN_EAST]=&quad[v*width + u+1];
            } else {
                neighbor[QN_EAST]=0;
            }

            // DEBUG: set neighbors to zero and see if Landscape still crashes
            //neighbor[QN_NORTH]=0;
            //neighbor[QN_SOUTH]=0;
            //neighbor[QN_EAST]=0;
            //neighbor[QN_WEST]=0;

            sprintf(buf,"%s-%d-%d.spr", texture_prefix.c_str(), u, v);
            main_tex = game->getTexMan()->query(
                    buf, JR_HINT_FULLOPACITY, 0, true);

            sprintf(buf,"%s-%d-%d.qad", terrain_prefix.c_str(), u, v);
            in.open(buf);
            if (!in) ls_error("LoDTerrain: Couldn't open %s\n", buf);
            sprintf(buf,"%s-%d-%d.tga", texmap_prefix.c_str(), u, v);
            char lbuf[256];
            sprintf(lbuf, "%s-%d-%d.spr", lightmap_prefix.c_str(), u, v);
            quad[v*width + u].init(the_game, in, neighbor, main_tex,
                    detail_tex, buf, textures, lbuf, stat);
            in.close();

            stat.stepFinished();
        }
    }
    stat.endJob();

    counter=0;

    game->getEventRemapper()->map("debug",
            SigC::slot(*this, &LoDQuadManager::toggleDebugMode));
    debug_mode = false;

    loadTextures();
}

LoDQuadManager::~LoDQuadManager()
{
    for (int i=0; i<(width*height); i++) {
        quad[i].done();
    }
    delete[] quad;
}

void LoDQuadManager::toggleDebugMode()
{
    debug_mode = !debug_mode;
}

// Begin: IDrawable method
void LoDQuadManager::draw()
{
    int i;
    float planes[6][4];

    camera->getFrustumPlanes(planes);
    Vector pos = camera->getLocation();

    for (i=0; i<(width*height); i++) {
        quad[i].presetup();
    }
    for (i=0; i<(width*height); i++) {
        quad[i].setup(pos, planes);
    }
    for (i=0; i<(width*height); i++) {
        quad[i].draw(renderer);
        if (debug_mode) quad[i].drawWire(renderer);
    }
    counter++;
}


// BEGIN: ITerrain method
float LoDQuadManager::getHeightAt(float x, float z)
{
    LoDQuad *q = getQuadAtPoint(x,z);
    if (q) {
        return q->getHeightAt(x,z);
    } else return 0.0;
}

namespace {
    // next_edge assumes that (px,py) is contained in the rectangle (x,y,w,h).
    // Then it tries to walk from there in direction (vx, vy) until it hits an
    // edge of the rectangle. Then it returns true and sets t to the parameter that
    // it found: (px, py) + t*(vx,vy) lies on an edge
    // If it can't hit an edge (since vx and vy are zero) it returns false.

    bool next_edge( float x, float y, float w, float h,
            float px, float py, float vx, float vy, float *t )
    {
        if (vx!=0 && vy!=0) {
            float tx = (x - px) / vx;
            if (tx < 0) tx = (x + w - px) / vx;
            float ty = (y - py) / vy;
            if (ty < 0) ty = (y + h - py) / vy;
            *t = std::min(tx, ty);
            return true;
        } else if ( vx!=0 ) {
            float tx = (x - px) / vx;
            if (tx < 0) tx = (x + w - px) / vx;
            *t = tx;
            return true;
        } else if ( vy!=0 ) {
            float ty = (y - py) / vy;
            if (ty < 0) ty = (y + h - py) / vy;
            *t = ty;
            return true;
        } else return false;
    }
}

// lineCollides tests whether an object going from a to b hits the terrain and
// returns the point where this happens

// bool LoDQuadManager::lineCollides(Vector a, Vector b, Vector * cx) {
//     LoDQuad *quad = getQuadAtPoint(a[0], a[2]);
//
//     if (!quad) return false;
//
//     float x = quad->vx[quad->triangle[0].vertex[2]];
//     float z = quad->vz[quad->triangle[0].vertex[2]];
//     float w = quad->vx[quad->triangle[1].vertex[2]] - x;
//     float h = quad->vz[quad->triangle[1].vertex[2]] - z;
//
//     Vector d = (b-a);
//
//     float t;
//
//     Vector p0;
//     Vector p1 = a;
//     Vector m = p1;
//     for( ; ; ) { // Exits via return
//         if (!next_edge( x, z, w, h, m[0], m[2], d[0], d[2], &t)) return false;
//         p0 = p1;
//         p1 += t * d;
//
//         if (((p0-a)*d) / d.lengthSquare() >= 1) return false;
//         if (((p1-a)*d) / d.lengthSquare() > 1) p1 = a+d;
//
//         m = (p1 + p0)/2;
//         quad = getQuadAtPoint(m[0], m[2]);
//         if (quad) {
//             float t0, t1;
//             bool c0, c1;
//             c0 = quad->lineCollides(p0, p1, &t0, &quad->triangle[0]);
//             c1 = quad->lineCollides(p0, p1, &t1, &quad->triangle[1]);
//             if (c0 || c1) {
//                 if (c0 && c1) t = min(t0, t1);
//                 else if (c0) t = t0;
//                 else t = t1;
//                 *cx = p0 + t*(p1-p0);
//             }
//         } else return false;
//         x = quad->vx[quad->triangle[0].vertex[2]];
//         z = quad->vz[quad->triangle[0].vertex[2]];
//     }
// }

bool LoDQuadManager::lineCollides(Vector a, Vector b, Vector * cx) {
    float t;
    static int count = 0;
    //ls_message("lineCollide for the %dth time.\n", ++count);
    if (getHeightAt(a[0],a[2]) >= a[1]) {
        *cx = a;
        return true;
    }

    LoDQuad * quad = getQuadAtPoint(a[0], a[2]);
    if (!quad) return false;
    if (quad->lineCollides(a,b, &t, &quad->triangle[0]) ||
        quad->lineCollides(a,b, &t, &quad->triangle[1])) {
        *cx = a + (b-a)*t;
        //game->drawDebugTriangleAt(*cx);
        return true;
    }
}


// Returns the quad that lies under the given X/Z-Pair.
// Returns 0 if there is none
LoDQuad * LoDQuadManager::getQuadAtPoint(float x, float z)
{
    // northwest point of landscape
    float qx=quad[0].vx[quad[0].triangle[0].vertex[2]];
    float qz=quad[0].vz[quad[0].triangle[0].vertex[2]];
    // tile width and length
    float dx=quad[0].vx[quad[0].triangle[0].vertex[1]] - qx;
    float dz=quad[0].vz[quad[0].triangle[0].vertex[0]] - qz;

    float uf = (x - qx) / dx;
    float vf = (z - qz) / dz;
    int u = (int)uf;
    int v = (int)vf;

    if ((u>=0)&&(u<width)&&(v>=0)&&(v<height)) {
        int idx = v*width + u;
        return &quad[idx];
    } else return 0;
}


void LoDQuadManager::loadTextures()
{
    std::string texfile = cfg->query("LoDQuadManager_texture_list");
    std::string basename = cfg->query("LoDQuadManager_tile_dir");

    std::ifstream in(texfile.c_str());
    int lines;
    in >> lines;
    std::string token;
    std::string prefix;
    std::string filename;
    for(int i=0; i<lines; i++) {
        in >> token;
        if (token == "single") {
            in >> prefix;
            filename = basename + "/" + prefix + ".spr";
            ls_message("loading texture %s\n", filename.c_str());
            textures[i][15] = game->getTexMan()->
                query(filename.c_str(), JR_HINT_FULLOPACITY, 0, true);
        } else if (token == "group") {
            in >> prefix;
            for (int j=1; j<16; j++) {
                std::string postfix = "-";
                for(int k=8;k!=0; k>>=1) postfix += j&k?'1':'0';
                postfix += ".spr";
                filename = basename + "/" + prefix + postfix;
                ls_message("loading texture %s\n", filename.c_str());
                textures[i][j] = game->getTexMan()->
                    query(filename.c_str(),
                        j==15?JR_HINT_FULLOPACITY:0, 0, true);
            }
        } else if (token == "skip") {
            ls_message("Skipping texture %d\n", i);
        } else {
            ls_error("LodQuadManager: illegal token %s in %f - aborting.\n",
                    token.c_str(), texfile.c_str());
            return;
        }
    }
}
