#include <fstream.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <landscape.h>

class Tile;

class Terrain: public ITerrain
{
    friend class Tile;
public:
    Terrain(IGame *thegame);

    virtual void draw();

    virtual float getHeightAt(float x, float z);
    virtual bool lineCollides(Vector a, Vector b, Vector * x);

private:
    void getCurrentHFPixel(int *u, int *v);
    
private:
    IGame *thegame;
    Ptr<IConfig> config;
    Ptr<ICamera> camera;
    JRenderer *renderer;
    jrtxtid_t txtid[20];
    ju8 text[513*513];
    float height[513*513];
    ju8 shade[513*513];
    float palette[256][3];
    Tile *tiles[TILE_NUM];
};

class Tile
{
public:
    Tile(Terrain *terrain, int u, int v);

    int getState(const Vector & pos_camera, float frustum[6][4]);
    void draw(Terrain *terrain, JRenderer *renderer,
              int state, int state_mu, int state_pu,
              int state_mv, int state_pv);

protected:
    void prepareVertices(float *height,jvertex_coltxt vrt[2][2], int u, int v);
    void prepareVertices2(jvertex_coltxt vrt[2][2], int u, int v);
    void colorizeVertices(Terrain *terrain, jvertex_coltxt vrt[2][2], int u, int v);
    void drawBlendedQuad(Terrain *terrain, JRenderer *renderer,
                         jvertex_coltxt vrt[2][2], int u, int v);
    void drawTexturedQuad(Terrain *terrain, JRenderer *renderer,
                          jvertex_coltxt vrt[2][2], int u, int v);
    void drawGouraudQuad(Terrain *terrain, JRenderer *renderer,
                         jvertex_coltxt vrt[2][2], int u, int v);
    
    void findSphere();
    void scaleDown(Terrain *terrain);
    void connectEdges(float *height, int where);
    
protected:
    float height[(TILE_SIZE+1)*(TILE_SIZE+1)];
    ju8 text[(TILE_SIZE+1)*(TILE_SIZE+1)];
    ju8 shade[(TILE_SIZE+1)*(TILE_SIZE+1)];
    float height2[(TILE_SIZE/2+1)*(TILE_SIZE/2+1)];
    ju8 text2[(TILE_SIZE/2+1)*(TILE_SIZE/2+1)];
    ju8 shade2[(TILE_SIZE/2+1)*(TILE_SIZE/2+1)];
    JPoint pos;
    int u_start;
    int v_start;
    float radius;
};

