#include <fstream>
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <modules/jogi/jogi.h>
#include <remap.h>
#include "map.h"

#define MAP_SIZE 0.15

using namespace std;

Map::Map(IGame *thegame)
:   thegame(thegame)
{
    ifstream in;
    JSprite spr;
    Ptr<IConfig> config( thegame->getConfig() );
    const char *texture_file=config->query("Map_texture_file");

    this->renderer=thegame->getRenderer();
    enabled=true;
    magnified=false;
    t=0.0f;

    in.open(texture_file);
    if (!in) ls_error("Map: Couldn't open %s\n", texture_file);
    spr.load(in);
    in.close();
    renderer->createTexture(&spr.sprite,0,0,false,&txtid);

    env = thegame->getEnvironment();

    EventRemapper *r = thegame->getEventRemapper();
    r->map("map_magnify", SigC::slot(*this, & Map::magnify));
    r->map("map_demagnify", SigC::slot(*this, & Map::demagnify));
}

void Map::enable()
{
    enabled=true;
}

void Map::disable()
{
    enabled=false;
}

void Map::magnify()
{
    magnified=true;
}

void Map::demagnify()
{
    magnified=false;
}

void Map::draw()
{
    JPoint pos,vec;
    jpoint_t vx,vy,vz;
    jmatrix_t m;
    JPoint p1(-MAP_SIZE, MAP_SIZE,0.0f);
    JPoint p2( MAP_SIZE, MAP_SIZE,0.0f);
    JPoint p3(-MAP_SIZE,-MAP_SIZE,0.0f);
    JPoint p4( MAP_SIZE,-MAP_SIZE,0.0f);
    jvertex_txt v1,v2,v3,v4;
    jpoint_t move1;
    jpoint_t move2={-0.75f,0.55f,1.1f};

    if (!enabled) {
        return;
    }

    if (magnified && (t < 1.0f)) {
        t+=0.2f;
    }

    if (!magnified && (t > 0.0f)) {
        t-=0.2f;
    }

    if (t < 0.0f) t=0.0f;
    if (t > 1.0f) t=1.0f;

    // Ptr<IPositionProvider> cam_pos = thegame->getCamPos();
    // if (!cam_pos) cam_pos = thegame->getCamera();
    Ptr<IPositionProvider> cam_pos = thegame->getCamera();
    Vector p = cam_pos->getLocation();
    Vector front = cam_pos->getFrontVector();
    pos.point.x = p[0];
    pos.point.y = p[1];
    pos.point.z = p[2];
    vec.point.x = front[0];
    vec.point.y = front[1];
    vec.point.z = front[2];


    vz.x=0.0f;
    vz.y=0.0f;
    vz.z=1.0f;

    vec.point.y=0;
    vec.normalize();

    vy.x=vec.point.x;
    vy.y=vec.point.z;
    vy.z=0.0f;

    vx.x=vy.y;
    vx.y=-vy.x;
    vx.z=0.0f;

    m.m[0][0]=vx.x;
    m.m[0][1]=vx.y;
    m.m[0][2]=vx.z;
    m.m[0][3]=0.0f;

    m.m[1][0]=vy.x;
    m.m[1][1]=vy.y;
    m.m[1][2]=vy.z;
    m.m[1][3]=0.0f;

    m.m[2][0]=vz.x;
    m.m[2][1]=vz.y;
    m.m[2][2]=vz.z;
    m.m[2][3]=0.0f;

    m.m[3][0]=0.0f;
    m.m[3][1]=0.0f;
    m.m[3][2]=0.0f;
    m.m[3][3]=1.0f;

    move1.x=-MAP_SIZE*pos.point.x/(TERRAIN_LENGTH/2.0f);
    move1.y=-MAP_SIZE*pos.point.z/(TERRAIN_LENGTH/2.0f);
    move1.z=0.0f;

    p1.add(&move1);
    p2.add(&move1);
    p3.add(&move1);
    p4.add(&move1);

    p1.scale(3.0f*t+1.0f);
    p2.scale(3.0f*t+1.0f);
    p3.scale(3.0f*t+1.0f);
    p4.scale(3.0f*t+1.0f);


    p1.applyMatrix(&m);
    p2.applyMatrix(&m);
    p3.applyMatrix(&m);
    p4.applyMatrix(&m);

    move2.x*=1.0f-t;
    move2.y*=1.0f-t;

    p1.add(&move2);
    p2.add(&move2);
    p3.add(&move2);
    p4.add(&move2);

    v1.p=p1.point;
    v2.p=p2.point;
    v3.p=p3.point;
    v4.p=p4.point;

    v1.txt.x=0.0f;
    v1.txt.y=0.0f;

    v2.txt.x=255.5f;
    v2.txt.y=0.0f;

    v3.txt.x=0.0f;
    v3.txt.y=255.5f;

    v4.txt.x=255.5f;
    v4.txt.y=255.5f;

    renderer->setCoordSystem(JR_CS_EYE);
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->setVertexMode(JR_VERTEXMODE_TEXTURE);
    renderer->enableAlphaBlending();
    renderer->setTexture(txtid);
    renderer->setClipRange(0.1,10.0);
    renderer->disableZBuffer();

    renderer->begin(JR_DRAWMODE_TRIANGLES);
    {
        renderer->addVertex(&v1);
        renderer->addVertex(&v2);
        renderer->addVertex(&v3);

        renderer->addVertex(&v3);
        renderer->addVertex(&v2);
        renderer->addVertex(&v4);
    }
    renderer->end();
    drawPlayerSymbol(move2.x,move2.y,move2.z);

    renderer->enableZBuffer();
    renderer->setClipRange(env->getClipMin(), env->getClipMax());
    renderer->disableAlphaBlending();
    //drawPlayerSymbol(move2.x,move2.y,move2.z);
    renderer->setCoordSystem(JR_CS_WORLD);

}


void Map::drawPlayerSymbol(float x, float y, float z)
{
    jvertex_col v1,v2,v3;

    v1.p.x=x;
    v1.p.y=y;
    v1.p.z=z;

    v2.p.x=v1.p.x+0.015f;
    v2.p.y=v1.p.y-0.03f;
    v2.p.z=v1.p.z-0.005f;

    v3.p.x=v1.p.x-0.015f;
    v3.p.y=v1.p.y-0.03f;
    v3.p.z=v1.p.z-0.005f;

    v1.col.r=255.0f;
    v1.col.g=255.0f;
    v1.col.b=0.0f;

    v2.col.r=255.0f;
    v2.col.g=255.0f;
    v2.col.b=0.0f;

    v3.col.r=255.0f;
    v3.col.g=255.0f;
    v3.col.b=0.0f;

    renderer->setVertexMode(JR_VERTEXMODE_GOURAUD);
    renderer->begin(JR_DRAWMODE_TRIANGLES);
    {
        renderer->addVertex(&v1);
        renderer->setAlpha(0);
        renderer->addVertex(&v2);
        renderer->addVertex(&v3);
    }
    renderer->end();
}
