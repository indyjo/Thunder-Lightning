#ifdef ENABLE_HORIZON
#include "horizon.h"

#define PI 3.14159265358979323846264338327

Horizon::Horizon(IGame *thegame)
{
    this->thegame=thegame;
    renderer = thegame->getRenderer();
    player = thegame->getPlayer();
    sky = thegame->getSky();
}

void Horizon::draw()
{
    jpoint_t pos;
    jvertex_col vtx_top0,vtx_top1,vtx_mid0,vtx_mid1;
    jvertex_col vtx_zero0,vtx_zero1,vtx_bot0,vtx_bot1;
    jcolor3_t sky_col;
    int i;
    float height_top, height_mid, height_zero, height_bot;
    float angle1, angle2;
    jBool restore_fog;
    JRenderer *renderer;

    renderer=thegame->getRenderer();
    player->getPosition(&pos);
    sky->getSkyColor(&sky_col);

    vtx_top0.col.r=sky_col.r;
    vtx_top0.col.g=sky_col.g;
    vtx_top0.col.b=sky_col.b;
    vtx_top1.col.r=sky_col.r;
    vtx_top1.col.g=sky_col.g;
    vtx_top1.col.b=sky_col.b;
    vtx_mid0.col.r=FOG_MIDCOLOR_RED;
    vtx_mid0.col.g=FOG_MIDCOLOR_GREEN;
    vtx_mid0.col.b=FOG_MIDCOLOR_BLUE;
    vtx_mid1.col.r=FOG_MIDCOLOR_RED;
    vtx_mid1.col.g=FOG_MIDCOLOR_GREEN;
    vtx_mid1.col.b=FOG_MIDCOLOR_BLUE;
    vtx_zero0.col.r=FOG_COLOR_RED;
    vtx_zero0.col.g=FOG_COLOR_GREEN;
    vtx_zero0.col.b=FOG_COLOR_BLUE;
    vtx_zero1.col.r=FOG_COLOR_RED;
    vtx_zero1.col.g=FOG_COLOR_GREEN;
    vtx_zero1.col.b=FOG_COLOR_BLUE;
    vtx_bot0.col.r=FOG_COLOR_RED;
    vtx_bot0.col.g=FOG_COLOR_GREEN;
    vtx_bot0.col.b=FOG_COLOR_BLUE;
    vtx_bot1.col.r=FOG_COLOR_RED;
    vtx_bot1.col.g=FOG_COLOR_GREEN;
    vtx_bot1.col.b=FOG_COLOR_BLUE;

    height_top=HORIZON_TOP_HEIGHT;
    height_mid=HORIZON_MID_HEIGHT;
    height_zero=HORIZON_ZERO_HEIGHT;
    height_bot=HORIZON_BOT_HEIGHT;

    vtx_top0.p.y=height_top;
    vtx_top1.p.y=height_top;
    vtx_mid0.p.y=height_mid;
    vtx_mid1.p.y=height_mid;
    vtx_zero0.p.y=height_zero;
    vtx_zero1.p.y=height_zero;
    vtx_bot0.p.y=height_bot;
    vtx_bot1.p.y=height_bot;

    restore_fog=renderer->fogEnabled();
    renderer->disableFog();
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->setVertexMode(JR_VERTEXMODE_GOURAUD);
    renderer->setClipRange(0.1f, HORIZON_RADIUS * 100.0f);
    renderer->disableZBuffer();

    renderer->begin(JR_DRAWMODE_TRIANGLES);
    
    for (i=0;i<HORIZON_SEGMENTS;i++) {
        angle1=(float) i;
        angle1/=(float) HORIZON_SEGMENTS;
        angle1*= 2.0f * PI;
        angle2=(float) ((i+1) % HORIZON_SEGMENTS);
        angle2/=(float) HORIZON_SEGMENTS;
        angle2*= 2.0f * PI;
        //ls_message("angle1: %f angle2:%f\n",angle1,angle2);
        
        vtx_top0.p.x=HORIZON_RADIUS * sin(angle1) + pos.x;
        vtx_mid0.p.x=vtx_top0.p.x;
        vtx_zero0.p.x=vtx_top0.p.x;
        vtx_bot0.p.x=vtx_top0.p.x;
        
        vtx_top1.p.x=HORIZON_RADIUS * sin(angle2) + pos.x;
        vtx_mid1.p.x=vtx_top1.p.x;
        vtx_zero1.p.x=vtx_top1.p.x;
        vtx_bot1.p.x=vtx_top1.p.x;

        vtx_top0.p.z=HORIZON_RADIUS * cos(angle1) + pos.z;
        vtx_mid0.p.z=vtx_top0.p.z;
        vtx_zero0.p.z=vtx_top0.p.z;
        vtx_bot0.p.z=vtx_top0.p.z;

        vtx_top1.p.z=HORIZON_RADIUS * cos(angle2) + pos.z;
        vtx_mid1.p.z=vtx_top1.p.z;
        vtx_zero1.p.z=vtx_top1.p.z;
        vtx_bot1.p.z=vtx_top1.p.z;

        renderer->addVertex(&vtx_top0);
        renderer->addVertex(&vtx_top1);
        renderer->addVertex(&vtx_mid0);
        renderer->addVertex(&vtx_mid0);
        renderer->addVertex(&vtx_top1);
        renderer->addVertex(&vtx_mid1);

        renderer->addVertex(&vtx_mid0);
        renderer->addVertex(&vtx_mid1);
        renderer->addVertex(&vtx_zero0);
        renderer->addVertex(&vtx_zero0);
        renderer->addVertex(&vtx_mid1);
        renderer->addVertex(&vtx_zero1);

        renderer->addVertex(&vtx_zero0);
        renderer->addVertex(&vtx_zero1);
        renderer->addVertex(&vtx_bot0);
        renderer->addVertex(&vtx_bot0);
        renderer->addVertex(&vtx_zero1);
        renderer->addVertex(&vtx_bot1);
    }

    renderer->end();
    renderer->flush();
    if (restore_fog) {
        renderer->enableFog();
    }
    renderer->setClipRange(1.0, CLIP_RANGE);
    renderer->enableZBuffer();
}
#endif
