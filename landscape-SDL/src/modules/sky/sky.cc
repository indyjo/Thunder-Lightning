#include "sky.h"

#define SKY_SEGMENTS_TO_DRAW 4
#define SKY_UV_TO_WORLD(u) (float)SKY_SEGMENT_SIZE*(u);
#define PI 3.14159265358979323846264338327

Sky::Sky(IGame *thegame)
{
    JSprite spr;
    ifstream in;
    jError ret;
    const char *filename;
    Ptr<IConfig> config( thegame->getConfig() );
    
    this->thegame=thegame;
    renderer=thegame->getRenderer();
    player=Ptr<IPlayer>( thegame->getPlayer() );

    filename=config->query("Sky_downside_texture_path");
    txtid1 = thegame->getTexMan()->query(filename, JR_HINT_GREYSCALE,0,true);
    filename=config->query("Sky_upside_texture_path");
    txtid2 = thegame->getTexMan()->query(filename, JR_HINT_GREYSCALE,0,true);
}


void Sky::draw()
{
    renderer->enableAlphaBlending();
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->setVertexMode(JR_VERTEXMODE_TEXTURE);

    Vector pos = player->getLocation();
    if (pos[1] < SKY_HEIGHT) {
        //drawL2();
        draw(txtid1);
    } else {
        draw(txtid2);
        //drawL2();
    }
    renderer->disableAlphaBlending();
}


void Sky::getCurrentUV(int *u, int *v)
{
    Vector pos = player->getLocation();
    *u=(int) (pos[0]/SKY_SEGMENT_SIZE);
    *v=(int) (pos[2]/SKY_SEGMENT_SIZE);
}

void Sky::draw(int txt)
{
    int su,eu,sv,ev,u,v;
    jvertex_txt vrt[2][2];

    getCurrentUV(&u,&v);
    su=u-SKY_SEGMENTS_TO_DRAW;
    eu=u+SKY_SEGMENTS_TO_DRAW;
    sv=v-SKY_SEGMENTS_TO_DRAW;
    ev=v+SKY_SEGMENTS_TO_DRAW;

    vrt[0][0].txt.x=0.5f;
    vrt[0][0].txt.y=0.5f;
    vrt[0][1].txt.x=255.5f;
    vrt[0][1].txt.y=0.5f;
    vrt[1][0].txt.x=0.5f;
    vrt[1][0].txt.y=255.5f;
    vrt[1][1].txt.x=255.5f;
    vrt[1][1].txt.y=255.5f;

    vrt[0][0].p.y=SKY_HEIGHT;
    vrt[0][1].p.y=SKY_HEIGHT;
    vrt[1][0].p.y=SKY_HEIGHT;
    vrt[1][1].p.y=SKY_HEIGHT;
    

    renderer->setTexture(txt);


    for (v=sv;v<=ev;v++) {
        vrt[0][0].p.z=SKY_UV_TO_WORLD(v);
        vrt[0][1].p.z=SKY_UV_TO_WORLD(v);
        vrt[1][0].p.z=SKY_UV_TO_WORLD(v+1);
        vrt[1][1].p.z=SKY_UV_TO_WORLD(v+1);

        for (u=su;u<=eu;u++) {
            vrt[0][0].p.x=SKY_UV_TO_WORLD(u);
            vrt[0][1].p.x=SKY_UV_TO_WORLD(u+1);
            vrt[1][0].p.x=SKY_UV_TO_WORLD(u);
            vrt[1][1].p.x=SKY_UV_TO_WORLD(u+1);

            /*ls_message("vrt[0][0]: x=%f y=%f z=%f r=%f g=%f b=%f\n",
             vrt[0][0].p.x,vrt[0][0].p.y,vrt[0][0].p.z,
             vrt[0][0].col.r,vrt[0][0].col.g,vrt[0][0].col.b);*/

            renderer->begin(JR_DRAWMODE_TRIANGLES);
            {
                renderer->addVertex(&vrt[0][0]);
                renderer->addVertex(&vrt[1][0]);
                renderer->addVertex(&vrt[1][1]);
                renderer->addVertex(&vrt[1][1]);
                renderer->addVertex(&vrt[0][1]);
                renderer->addVertex(&vrt[0][0]);
            }
            renderer->end();
        }
    }
    renderer->flush();
}




void Sky::getSkyColor(jcolor3_t *col)
{
    float t,s;

    Vector pos = player->getLocation();

    if (pos[1] <= SKY_BLUE_HEIGHT) {
        col->r=SKY_COLOR_RED;
        col->g=SKY_COLOR_GREEN;
        col->b=SKY_COLOR_BLUE;
    } else if (pos[1] >= SKY_BLACK_HEIGHT) {
        col->r=0.0f;
        col->g=0.0f;
        col->b=0.0f;
    } else {
        t=1-(pos[1]-SKY_BLUE_HEIGHT)/(SKY_BLACK_HEIGHT-SKY_BLUE_HEIGHT);
        t=t * PI -PI/2.0f; // auf -PI/2 bis +PI/2 strecken
        s=(sin(t)+1.0f)/2.0f; // von -1 bis +1 auf 0 bis 1 bringen
        col->r=s*SKY_COLOR_RED;
        col->g=s*SKY_COLOR_GREEN;
        col->b=s*SKY_COLOR_BLUE;
    }

    return;
}
        
