#include <cstdio>
#include <cmath>
#include <fstream>
#include "skybox.h"
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>

static const char * tex_names[6] = {
    "front.png", "left.png", "right.png", "back.png", "up.png", "down.png"
};

/*
      3--2
     /| /|
    0--1 |
    | 7|-6
    |/ |/
    4--5
*/
    
static const float points[8][3] = {
    {-1, 1,-1},
    { 1, 1,-1},
    { 1, 1, 1},
    {-1, 1, 1},
    {-1,-1,-1},
    { 1,-1,-1},
    { 1,-1, 1},
    {-1,-1, 1}};

static const int indices[6][4] = {
    {3,2,6,7},
    {0,3,7,4},
    {2,1,5,6},
    {1,0,4,5},
    {0,1,2,3},
    {7,6,5,4}};

SkyBox::SkyBox(IGame *thegame)
{
    std::ifstream in;
    char filename[255];
    const char *texpath;
    this->thegame=thegame;
    renderer=thegame->getRenderer();
    config=thegame->getConfig();
    Ptr<TextureManager> texman = thegame->getTexMan();
    
    texpath=config->query("SkyBox_texture_path");
    
    for(int i=0; i<6; i++) {
        sprintf(filename, "%s/%s", texpath, tex_names[i]);
        ls_message("SkyBox: loading texture %s\n", filename);
        textures[i] = texman->query(filename, JR_HINT_FULLOPACITY,0,false);
    }
}

void SkyBox::draw()
{
    jBool restore_fog;

    Vector p = thegame->getCamera()->getLocation();

    restore_fog=renderer->fogEnabled();
    renderer->disableFog();
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->setVertexMode(JR_VERTEXMODE_GOURAUD_TEXTURE);
    renderer->disableZBuffer();
    
    renderer->enableTexturing();
    renderer->enableSmoothShading();
    renderer->disableAlphaBlending();
    
    float D = thegame->getEnvironment()->getClipMax() / 1.75;
    
    Vector uvw[4];
    uvw[0] = Vector(0,1,0);
    uvw[1] = Vector(1,1,0);
    uvw[2] = Vector(1,0,0);
    uvw[3] = Vector(0,0,0);
    
    for(int i=0; i<6; i++) {
        renderer->setTexture(textures[i]->getTxtid());
        renderer->setWrapMode(JR_TEXDIM_U, JR_WRAPMODE_CLAMP);
        renderer->setWrapMode(JR_TEXDIM_V, JR_WRAPMODE_CLAMP);
        renderer->setWrapMode(JR_TEXDIM_W, JR_WRAPMODE_CLAMP);
        renderer->begin(JR_DRAWMODE_TRIANGLE_FAN);
        renderer->setColor(Vector(1,1,1));
        for(int j=0; j<4; j++) {
            int idx = indices[i][j];
            renderer->setUVW(uvw[j]);
            Vector v(points[idx][0], points[idx][1], points[idx][2]);
            renderer->vertex(p+D*v);
        }
        renderer->end();
    }
    
    if (restore_fog) {
        renderer->enableFog();
    }
    renderer->enableZBuffer();
}
