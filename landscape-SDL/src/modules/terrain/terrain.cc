#include "terrain.h"

#define WIDTH_OF_HFPIXEL TERRAIN_LENGTH/512.0f
//#define HFPIXELS_TO_DRAW_F (CLIP_RANGE/WIDTH_OF_HFPIXEL+1.0f)
//#define HFPIXELS_TO_DRAW (int) HFPIXELS_TO_DRAW_F
//#define HFPIXELS_TO_DRAW 16
#define HF_STARTING_POINT (-TERRAIN_LENGTH/2.0f)
#define U_TO_WORLD(u) ((float)HF_STARTING_POINT+TERRAIN_LENGTH*(u)/512.0f)
#define V_TO_WORLD(v) ((float)HF_STARTING_POINT+TERRAIN_LENGTH*(512.0f-(v))/512.0f)
#define WAVE_SPEED 0.001f
#define WAVE_HEIGHT 10.0f
// The squared range in which terrain quads are displayed blended (smooth)
#define BLENDED_TERRAIN_RANGE 1500
//#define BLENDED_TERRAIN_RANGE -1000
// The squared range in which terrain quads are displayed textured
#define TEXTURED_TERRAIN_RANGE 2000
//#define TEXTURED_TERRAIN_RANGE -1000

#define TILES_TO_DRAW 6

#define PI 3.14159265358979323846264338327

#define EDGE_MINUS_V 1
#define EDGE_PLUS_V 2
#define EDGE_MINUS_U 4
#define EDGE_PLUS_U 8

using namespace std;


Terrain::Terrain(IGame *thegame)
{
    int i,j,u,v;
    unsigned char data;
    JSprite spr;
    ifstream in;
    jError ret;
    char name[256];
    int red,green,blue;
    char buffer[256];
    const char *data_dir, *tex_dir;
    
    this->thegame=thegame;
    renderer=thegame->getRenderer();
    config=thegame->getConfig();
    camera=thegame->getCamera();

    /*ls_message("TERRAIN_LENGTH=%f\n",TERRAIN_LENGTH);
    ls_message("WIDTH_OF_HFPIXEL=%f\n",WIDTH_OF_HFPIXEL);
    ls_message("HFPIXELS_TO_DRAW=%d\n",HFPIXELS_TO_DRAW);
    ls_message("CLIP_RANGE=%f\n",CLIP_RANGE);*/
            
    data_dir = config->query("data_dir");

    for (i=0;i<15;i++) {
        sprintf(name,"%s/textures/terrain-texture-%d.spr",data_dir, i);
        ls_message("[TERRAIN init: loading %s\n", name);
        in.open(name);
        spr.load(in);
        in.close();

        ret=renderer->createTexture(&spr.sprite,JR_HINT_FULLOPACITY,0,false,&txtid[i+5]);
        //ls_message("createTexture return code: %d\n",ret);
    }

    for (i=0;i<5;i++) {
        sprintf(name,"%s/textures/water-texture-%d.spr",data_dir, i);
        ls_message("[TERRAIN init: loading %s\n", name);
        in.open(name);
        spr.load(in);
        in.close();

        ret=renderer->createTexture(&spr.sprite,JR_HINT_FULLOPACITY,0,false,&txtid[i]);
        //ls_message("createTexture return code: %d\n",ret);
    }
    
    
    sprintf(name,"%s/terrains/hfl/elevation.tga", data_dir);
    ls_message("[TERRAIN init: loading %s\n", name);
    in.open(name);
    in.seekg(60,ios::beg);
    for(i=0;i<513;i++) {
        for (j=0;j<513;j++)
        {
            data=in.get();
            height[i*513 +j]=(float) data/255.0f*TERRAIN_HEIGHT;
        }
    }
    in.close();
    
    sprintf(name,"%s/terrains/hfl/shading.tga", data_dir);
    ls_message("[TERRAIN init: loading %s\n", name);
    in.open(name);
    in.seekg(60,ios::beg);
    //in.read(dummy,60);
    for(i=0;i<513;i++) {
        for (j=0;j<513;j++)
        {
            data=in.get();
            shade[i*513+j]=data;
        }
    }
    in.close();

    sprintf(name,"%s/terrains/hfl/texturemap.tga", data_dir);
    ls_message("[TERRAIN init: loading %s\n", name);
    in.open(name);
    in.seekg(828,ios::beg);
    //in.read(dummy,60);
    for(i=0;i<513;i++) {
        for (j=0;j<513;j++)
        {
            data=in.get();
            text[i*513+j]=data;
        }
    }
    in.close();

    sprintf(name,"%s/terrains/hfl/palette.dat", data_dir);
    ls_message("[TERRAIN init: loading %s\n", name);
    in.open(name);
    i=0;
    while (!in.eof()) {
        in.getline(buffer,255);
        ret=sscanf(buffer,"%x %x %x",&red,&green,&blue);
        //ls_message("ret:%d red=%d green=%d blue=%d\n",ret,red,green,blue);
        palette[i][0]=((float) red ) / 255.0f;
        palette[i][1]=((float) green ) / 255.0f;
        palette[i][2]=((float) blue ) / 255.0f;
        i++;
    }
    in.close();

    for (v=0;v<(512/TILE_SIZE);v++) {
        for (u=0;u<(512/TILE_SIZE);u++) {
            tiles[v*(512/TILE_SIZE)+u]=new Tile(this,u*TILE_SIZE,v*TILE_SIZE);
        }
    }
    
    ls_message("[TERRAIN init: finished]\n");
}

#define STATE_WIDTH ((TILES_TO_DRAW*2+1)*(TILES_TO_DRAW*2+1))

void Terrain::draw() {
    //ls_message("[TERRAIN: draw]\n");
    //ls_message("[  this = %p]\n", this);
    //ls_message("[  thegame = %p]\n", thegame);
    //ls_message("[  thegame->getRenderer() = %p]\n", thegame->getRenderer());
    //ls_message("[  STATE_WIDTH^2 = %d]\n", STATE_WIDTH * STATE_WIDTH);
    
    //static int *state = new int[STATE_WIDTH*STATE_WIDTH];
    //ls_message("[  state = %p\n", state);

    JRenderer *renderer=thegame->getRenderer();
    int u,v,u_start,u_end,v_start,v_end;
    int u_player,v_player;
    float time;
    float frustum[6][4];
    char state[STATE_WIDTH*STATE_WIDTH];
    int state_this,state_mu,state_pu,state_mv,state_pv;
    
    renderer->setCullMode(JR_CULLMODE_CULL_POSITIVE);

    Vector pos_camera = camera->getLocation();
    camera->getFrustumPlanes(frustum);
    time=(float) thegame->getTime();

    getCurrentHFPixel(&u_player,&v_player);
    u_player/=TILE_SIZE;
    v_player/=TILE_SIZE;
    
    u_start=u_player-TILES_TO_DRAW;
    v_start=v_player-TILES_TO_DRAW;
    u_end=u_player+TILES_TO_DRAW;
    v_end=v_player+TILES_TO_DRAW;

    if (u_start < 0) u_start=0;
    if (u_end >= (512/TILE_SIZE)) u_end=512/TILE_SIZE-1;
    if (v_start < 0) v_start=0;
    if (v_end >= (512/TILE_SIZE)) v_end=512/TILE_SIZE-1;

    /*
     We setup an array of Tile states before drawing so that every Tile knows
     what state its neighbors are drawn in so that it can connect its vertices
     properly
     */
    for (v=v_start;v<v_end;v++) {
        for (u=u_start;u<u_end;u++) {
            state[(v-v_start)*STATE_WIDTH+u-u_start]=
                tiles[v*(512/TILE_SIZE)+u]->getState(pos_camera,frustum);
        }
    }
    
    for (v=v_start;v<v_end;v++) {
        for (u=u_start;u<u_end;u++) {
            state_this=state[(v-v_start)*STATE_WIDTH+u-u_start];
            if (u>u_start) {
                state_mu=state[(v-v_start)*STATE_WIDTH+u-u_start-1];
            } else {
                state_mu=0;
            }
            if (u<(u_end-1)) {
                state_pu=state[(v-v_start)*STATE_WIDTH+u-u_start+1];
            } else {
                state_pu=0;
            }
            if (v>v_start) {
                state_mv=state[(v-v_start-1)*STATE_WIDTH+u-u_start];
            } else {
                state_mv=0;
            }
            if (v<(v_end-1)) {
                state_pv=state[(v-v_start+1)*STATE_WIDTH+u-u_start];
            } else {
                state_pv=0;
            }
            if (state) {
                tiles[v*(512/TILE_SIZE)+u]->
                    draw(this,renderer,
                         state_this,state_mu,state_pu,state_mv,state_pv);
            }
        }
    }
}


void Terrain::getCurrentHFPixel(int *u, int *v)
{
    Vector pos = camera->getLocation();
    *u=(int) ((pos[0]-HF_STARTING_POINT)*512.0f/TERRAIN_LENGTH);
    *v=(int) ((-pos[2]-HF_STARTING_POINT)*512.0f/TERRAIN_LENGTH);
}


float Terrain::getHeightAt(float x, float z)
{
    double u_float,v_float,u_fract,v_fract,u_int,v_int;
    float height00, height10, height01, height11;
    int      idx00,    idx10,    idx01,    idx11;
    int u,v;
    int wave_idx;
    float sin_result,time;
    float wave_strength[3]={1.0f,0.66f,0.33f};
    float u_world,v_world;

    u_float=(x-HF_STARTING_POINT) * 512.0f / TERRAIN_LENGTH;
    v_float=(-z-HF_STARTING_POINT) * 512.0f / TERRAIN_LENGTH;

    u_fract=modf(u_float,&u_int);
    v_fract=modf(v_float,&v_int);

    // Check if x,z are coords on the battlefield, if not return 0
    if ((u_float < 0.0f) || (u_float>=511.0f) ||
        (v_float < 0.0f) || (v_float>=511.0f))
    {
        return 0.0f;
    }

    time=(float) thegame->getTime();
    
    u=(int) u_int;
    v=(int) v_int;

    idx00=v*513+u;
    idx10=idx00+1;
    idx01=idx00+513;
    idx11=idx00+514;

    height00=height[idx00];
    height10=height[idx10];
    height01=height[idx01];
    height11=height[idx11];
    
    u_world=U_TO_WORLD(u_int);
    v_world=V_TO_WORLD(u_int);

    if (text[idx00]<3) {
        sin_result=sin( time * WAVE_SPEED + u_world*v_world);
        wave_idx=text[idx00];
        height00+=WAVE_HEIGHT*sin_result*wave_strength[wave_idx];
    }
        
    u_world=U_TO_WORLD(u_int+1.0f);
    
    if (text[idx10]<3) {
        sin_result=sin( time * WAVE_SPEED + u_world*v_world);
        wave_idx=text[idx10];
        height10+=WAVE_HEIGHT*sin_result*wave_strength[wave_idx];
    }

    v_world=V_TO_WORLD(v_int+1.0f);

    if (text[idx11]<3) {
        sin_result=sin( time * WAVE_SPEED + u_world*v_world);
        wave_idx=text[idx11];
        height11+=WAVE_HEIGHT*sin_result*wave_strength[wave_idx];
    }

    u_world=U_TO_WORLD(u_int);

    if (text[idx01]<3) {
        sin_result=sin( time * WAVE_SPEED + u_world*v_world);
        wave_idx=text[idx01];
        height01+=WAVE_HEIGHT*sin_result*wave_strength[wave_idx];
    }

    if (u_fract > v_fract) {
        return height00
            +u_fract*(height10 - height00)
            +v_fract*(height11 - height10);
    } else {
        return height[v*513+u]
            +u_fract*(height11 - height01)
            +v_fract*(height01 - height00);
    }
}

#define EPSILON 0.000001
#define MAX_INTERVAL_SQUARE 1.0
bool Terrain::lineCollides(Vector a, Vector b, Vector * x)
{
    float h_a = getHeightAt(a[0], a[2]);
    float h_b = getHeightAt(b[0], b[2]);
    
    if (h_a < a[1] && h_b < b[1]) return false;
    if (h_a >= a[1] && h_b >= b[1]) {
        *x=a;
        return true;
    }
    
    if ((a-b).lengthSquare() > MAX_INTERVAL_SQUARE) {
        return lineCollides(a, (a+b)/2.0, x) || lineCollides((a+b)/2.0, b, x);
    }
    
    float delta = (a-b).length();
    while(delta >= EPSILON) {
        Vector m = (a + b) / 2;
        float h_m = getHeightAt(m[0], m[2]);
        if (h_a >= a[1]) {
            if (h_m >= m[1]) {
                a = m;
                h_a = h_m;
            } else {
                b = m;
                h_b = h_m;
            }
        } else {
            if (h_m >= m[1]) {
                b = m;
                h_b = h_m;
            } else {
                a = m;
                h_a = h_m;
            }
        }
        delta /= 2;
    }
    
    *x = a;
    return true;
}


Tile::Tile(Terrain *terrain, int u, int v)
{
    int x,y;

    u_start=u;
    v_start=v;

    pos.point.x=U_TO_WORLD(u+TILE_SIZE/2);
    pos.point.z=V_TO_WORLD(v+TILE_SIZE/2);
    pos.point.y=0;

    for (y=0;y<=TILE_SIZE;y++) {
        for (x=0;x<=TILE_SIZE;x++) {
            height [y*(TILE_SIZE+1)+x] = terrain->height [(y+v)*513+x+u];
            shade  [y*(TILE_SIZE+1)+x] = terrain->shade  [(y+v)*513+x+u];
            text   [y*(TILE_SIZE+1)+x] = terrain->text   [(y+v)*513+x+u];
        }
    }

    findSphere();
    scaleDown(terrain);
}


int Tile::getState(const Vector & pos_camera, float frustum[6][4])
{
    float dist,detail_dist;

    dist=pos_camera[0]*frustum[PLANE_MINUS_Z][0]
        +pos_camera[1]*frustum[PLANE_MINUS_Z][1]
        +pos_camera[2]*frustum[PLANE_MINUS_Z][2]
        +frustum[PLANE_MINUS_Z][3];
    if (dist<(-radius)) return 0;
    detail_dist=dist;

    dist=pos_camera[0]*frustum[PLANE_PLUS_Z][0]
        +pos_camera[1]*frustum[PLANE_PLUS_Z][1]
        +pos_camera[2]*frustum[PLANE_PLUS_Z][2]
        +frustum[PLANE_PLUS_Z][3];
    if (dist<(-radius)) return 0;
    
    dist=pos_camera[0]*frustum[PLANE_MINUS_X][0]
        +pos_camera[1]*frustum[PLANE_MINUS_X][1]
        +pos_camera[2]*frustum[PLANE_MINUS_X][2]
        +frustum[PLANE_MINUS_X][3];
    if (dist<(-radius)) return 0;

    dist=pos_camera[0]*frustum[PLANE_PLUS_X][0]
        +pos_camera[1]*frustum[PLANE_PLUS_X][1]
        +pos_camera[2]*frustum[PLANE_PLUS_X][2]
        +frustum[PLANE_PLUS_X][3];
    if (dist<(-radius)) return 0;

    dist=pos_camera[0]*frustum[PLANE_MINUS_Y][0]
        +pos_camera[1]*frustum[PLANE_MINUS_Y][1]
        +pos_camera[2]*frustum[PLANE_MINUS_Y][2]
        +frustum[PLANE_MINUS_Y][3];
    if (dist<(-radius)) return 0;

    dist=pos_camera[0]*frustum[PLANE_PLUS_Y][0]
        +pos_camera[1]*frustum[PLANE_PLUS_Y][1]
        +pos_camera[2]*frustum[PLANE_PLUS_Y][2]
        +frustum[PLANE_PLUS_Y][3];
    if (dist<(-radius)) return 0;

    if (detail_dist<BLENDED_TERRAIN_RANGE) return 1;
    if (detail_dist<TEXTURED_TERRAIN_RANGE) return 2;
    return 3;
}


void Tile::draw(Terrain *terrain, JRenderer *renderer,
                int state, int state_mu, int state_pu,
                int state_mv, int state_pv)
{
    int u_end,v_end,u,v;
    int i;
    jvertex_coltxt vrt[2][2];
    float height_con[(TILE_SIZE+1)*(TILE_SIZE+1)];
    int edge_con;

    vrt[0][0].txt.x=0.0f;
    vrt[0][0].txt.y=0.0f;
    vrt[0][1].txt.x=64.0f;
    vrt[0][1].txt.y=0.0f;
    vrt[1][0].txt.x=0.0f;
    vrt[1][0].txt.y=64.0f;
    vrt[1][1].txt.x=64.0f;
    vrt[1][1].txt.y=64.0f;

    if (state==1) {
        edge_con=0;
        if (state_mu>2) edge_con|=EDGE_MINUS_U;
        if (state_pu>2) edge_con|=EDGE_PLUS_U;
        if (state_mv>2) edge_con|=EDGE_MINUS_V;
        if (state_pv>2) edge_con|=EDGE_PLUS_V;
        if (edge_con) {
            for (i=0;i<(TILE_SIZE+1)*(TILE_SIZE+1);i++) {
                height_con[i]=height[i];
            }
            connectEdges(height_con,edge_con);
            renderer->setVertexMode(JR_VERTEXMODE_GOURAUD_TEXTURE);
            for (v=0;v<TILE_SIZE;v++) {
                for (u=0;u<TILE_SIZE;u++) {
                    prepareVertices(height_con,vrt,u,v);
                    drawBlendedQuad(terrain,renderer,vrt,u,v);
                }
            }
        } else {
            renderer->setVertexMode(JR_VERTEXMODE_GOURAUD_TEXTURE);
            for (v=0;v<TILE_SIZE;v++) {
                for (u=0;u<TILE_SIZE;u++) {
                    prepareVertices(height,vrt,u,v);
                    drawBlendedQuad(terrain,renderer,vrt,u,v);
                }
            }
        }
    } else if (state==2) {
        edge_con=0;
        if (state_mu>2) edge_con|=EDGE_MINUS_U;
        if (state_pu>2) edge_con|=EDGE_PLUS_U;
        if (state_mv>2) edge_con|=EDGE_MINUS_V;
        if (state_pv>2) edge_con|=EDGE_PLUS_V;
        if (edge_con) {
            for (i=0;i<(TILE_SIZE+1)*(TILE_SIZE+1);i++) {
                height_con[i]=height[i];
            }
            connectEdges(height_con,edge_con);
            renderer->setVertexMode(JR_VERTEXMODE_GOURAUD_TEXTURE);
            for (v=0;v<TILE_SIZE;v++) {
                for (u=0;u<TILE_SIZE;u++) {
                    prepareVertices(height_con,vrt,u,v);
                    drawTexturedQuad(terrain,renderer,vrt,u,v);
                }
            }
        } else {
            renderer->setVertexMode(JR_VERTEXMODE_GOURAUD_TEXTURE);
            for (v=0;v<TILE_SIZE;v++) {
                for (u=0;u<TILE_SIZE;u++) {
                    prepareVertices(height,vrt,u,v);
                    drawTexturedQuad(terrain,renderer,vrt,u,v);
                }
            }
        }
    } else if (state==3) {
        renderer->setVertexMode(JR_VERTEXMODE_GOURAUD);
        for (v=0;v<(TILE_SIZE/2);v++) {
            for (u=0;u<(TILE_SIZE/2);u++) {
                prepareVertices2(vrt,u,v);
                colorizeVertices(terrain,vrt,u,v);
                drawTexturedQuad(terrain,renderer,vrt,u,v);
            }
        }
    }
}

void Tile::prepareVertices(float *height,jvertex_coltxt vrt[2][2], int u, int v)
{
    int idx00,idx01,idx10,idx11;
    float sin_result;
    int wave_idx;
    float wave_strength[3]={0.7f,0.5f,0.3f};
    float light_bias[3]={0.3f,0.5f,0.7f};

    vrt[0][0].p.z=V_TO_WORLD(v+v_start);
    vrt[0][1].p.z=V_TO_WORLD(v+v_start);
    vrt[1][0].p.z=V_TO_WORLD(v+v_start+1);
    vrt[1][1].p.z=V_TO_WORLD(v+v_start+1);

    vrt[0][0].p.x=U_TO_WORLD(u+u_start);
    vrt[0][1].p.x=U_TO_WORLD(u+u_start+1);
    vrt[1][0].p.x=U_TO_WORLD(u+u_start);
    vrt[1][1].p.x=U_TO_WORLD(u+u_start+1);

    idx00=v*(TILE_SIZE+1)+u;
    idx01=idx00+1;
    idx10=idx00+TILE_SIZE+1;
    idx11=idx10+1;
    vrt[0][0].p.y=height[idx00];
    vrt[0][1].p.y=height[idx01];
    vrt[1][0].p.y=height[idx10];
    vrt[1][1].p.y=height[idx11];

    vrt[0][0].col.r=shade[idx00];
    vrt[0][0].col.g=shade[idx00];
    vrt[0][0].col.b=shade[idx00];
    vrt[0][1].col.r=shade[idx01];
    vrt[0][1].col.g=shade[idx01];
    vrt[0][1].col.b=shade[idx01];
    vrt[1][0].col.r=shade[idx10];
    vrt[1][0].col.g=shade[idx10];
    vrt[1][0].col.b=shade[idx10];
    vrt[1][1].col.r=shade[idx11];
    vrt[1][1].col.g=shade[idx11];
    vrt[1][1].col.b=shade[idx11];

}


void Tile::prepareVertices2(jvertex_coltxt vrt[2][2], int u, int v)
{
    int idx00,idx01,idx10,idx11;
    float sin_result;
    int wave_idx;
    float wave_strength[3]={0.7f,0.5f,0.3f};
    float light_bias[3]={0.3f,0.5f,0.7f};

    vrt[0][0].p.z=V_TO_WORLD(2*v+v_start);
    vrt[0][1].p.z=V_TO_WORLD(2*v+v_start);
    vrt[1][0].p.z=V_TO_WORLD(2*v+v_start+2);
    vrt[1][1].p.z=V_TO_WORLD(2*v+v_start+2);

    vrt[0][0].p.x=U_TO_WORLD(2*u+u_start);
    vrt[0][1].p.x=U_TO_WORLD(2*u+u_start+2);
    vrt[1][0].p.x=U_TO_WORLD(2*u+u_start);
    vrt[1][1].p.x=U_TO_WORLD(2*u+u_start+2);

    idx00=v*(TILE_SIZE/2+1)+u;
    idx01=idx00+1;
    idx10=idx00+TILE_SIZE/2+1;
    idx11=idx10+1;
    vrt[0][0].p.y=height2[idx00];
    vrt[0][1].p.y=height2[idx01];
    vrt[1][0].p.y=height2[idx10];
    vrt[1][1].p.y=height2[idx11];

    vrt[0][0].col.r=shade2[idx00];
    vrt[0][0].col.g=shade2[idx00];
    vrt[0][0].col.b=shade2[idx00];
    vrt[0][1].col.r=shade2[idx01];
    vrt[0][1].col.g=shade2[idx01];
    vrt[0][1].col.b=shade2[idx01];
    vrt[1][0].col.r=shade2[idx10];
    vrt[1][0].col.g=shade2[idx10];
    vrt[1][0].col.b=shade2[idx10];
    vrt[1][1].col.r=shade2[idx11];
    vrt[1][1].col.g=shade2[idx11];
    vrt[1][1].col.b=shade2[idx11];

}


void Tile::colorizeVertices(Terrain *terrain, jvertex_coltxt vrt[2][2], int u, int v)
{
    int idx00,idx01,idx10,idx11;
    idx00=v*(TILE_SIZE/2+1)+u;
    idx01=idx00+1;
    idx10=idx00+TILE_SIZE/2+1;
    idx11=idx10+1;

    vrt[0][0].col.r*=terrain->palette[text2[idx00]][0];
    vrt[0][0].col.g*=terrain->palette[text2[idx00]][1];
    vrt[0][0].col.b*=terrain->palette[text2[idx00]][2];
    vrt[0][1].col.r*=terrain->palette[text2[idx01]][0];
    vrt[0][1].col.g*=terrain->palette[text2[idx01]][1];
    vrt[0][1].col.b*=terrain->palette[text2[idx01]][2];
    vrt[1][0].col.r*=terrain->palette[text2[idx10]][0];
    vrt[1][0].col.g*=terrain->palette[text2[idx10]][1];
    vrt[1][0].col.b*=terrain->palette[text2[idx10]][2];
    vrt[1][1].col.r*=terrain->palette[text2[idx11]][0];
    vrt[1][1].col.g*=terrain->palette[text2[idx11]][1];
    vrt[1][1].col.b*=terrain->palette[text2[idx11]][2];
}


void Tile::drawBlendedQuad(Terrain *terrain, JRenderer *renderer,
                           jvertex_coltxt vrt[2][2],int u, int v)
{
    /* Vertex order:
     0-1
     |\|
     2-3
     */
    int i,j;
    int idx[4];
    unsigned char texture[4],txt_list[4],cur_texture;
    int texture_count;
    int found;
    float alpha[4];

    idx[0]=v*(TILE_SIZE+1)+u;
    idx[1]=idx[0]+1;
    idx[2]=idx[0]+TILE_SIZE+1;
    idx[3]=idx[2]+1;

    for (i=0;i<4;i++)
    {
        texture[i]=text[idx[i]];
    }


    texture_count=0;
    for (i=0;i<4;i++)
    {
        found=false;
        for (j=0;j<i;j++)
        {
            if (texture[j]==texture[i]) {
                found=true;
                break;
            }
        }
        if (!found) {
            txt_list[texture_count]=texture[i];
            texture_count++;
        }
    }

    renderer->setTexture(terrain->txtid[txt_list[i]]);
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

    if (texture_count==1) {
        return;
    }

    renderer->enableAlphaBlending();
    renderer->setZBufferFunc(JR_ZBFUNC_EQUAL);

    for (i=1;i<texture_count;i++)
    {
        for (j=0;j<4;j++)
        {
            if (txt_list[i]==texture[j]) {
                alpha[j]=255.0f;
            } else {
                alpha[j]=0.0f;
            }
        }

        renderer->setTexture(terrain->txtid[txt_list[i]]);
        renderer->begin(JR_DRAWMODE_TRIANGLES);
        {
            renderer->setAlpha(alpha[0]);
            renderer->addVertex(&vrt[0][0]);
            renderer->setAlpha(alpha[2]);
            renderer->addVertex(&vrt[1][0]);
            renderer->setAlpha(alpha[3]);
            renderer->addVertex(&vrt[1][1]);
            renderer->addVertex(&vrt[1][1]);
            renderer->setAlpha(alpha[1]);
            renderer->addVertex(&vrt[0][1]);
            renderer->setAlpha(alpha[0]);
            renderer->addVertex(&vrt[0][0]);
        }
        renderer->end();
    }

    renderer->disableAlphaBlending();
    renderer->setZBufferFunc(JR_ZBFUNC_LESS);
}


void Tile::drawTexturedQuad(Terrain *terrain, JRenderer *renderer,
                            jvertex_coltxt vrt[2][2], int u, int v)
{
    renderer->setTexture(terrain->txtid[text[v*(TILE_SIZE+1)+u]]);
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


void Tile::drawGouraudQuad(Terrain *terrain, JRenderer *renderer,
                           jvertex_coltxt vrt[2][2], int u, int v)
{
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

void Tile::findSphere()
{
    JPoint dif;
    int u,v;
    float h=0.0f;
    float len, len_max;

    for (v=0;v<=TILE_SIZE;v++) {
        for (u=0;u<=TILE_SIZE;u++) {
            h+=height[v*(TILE_SIZE+1)+u];
        }
    }
    h/=(float) ((TILE_SIZE+1)*(TILE_SIZE+1));
    pos.point.y=h;

    len_max=0.0f;
    for (v=0;v<=TILE_SIZE;v++) {
        for (u=0;u<=TILE_SIZE;u++) {
            dif.set(U_TO_WORLD(u+u_start),
                    height[v*(TILE_SIZE+1)+u],
                    V_TO_WORLD(v+v_start));
            dif.sub(&pos.point);
            len=dif.getLength();
            if (len>len_max) {
                len_max=len;
            }
        }
    }
    radius=len_max;
}


void Tile::scaleDown(Terrain *terrain)
{
    int u,v,i,j;
    float height;
    int shade_avg,texture;

    for (v=0;v<=TILE_SIZE/2;v++) {
        for (u=0;u<=TILE_SIZE/2;u++) {
            shade_avg=0;
            for(j=0;j<2;j++) {
                for (i=0;i<2;i++) {
                    shade_avg+=terrain->shade[(v_start+v*2+j)*513+u_start+u*2+i];
                }
            }
            shade_avg/=4;

            height=terrain->height[(v_start+v*2)*513+u_start+u*2];
            texture=terrain->text[(v_start+v*2)*513+u_start+u*2];

            height2[v*(TILE_SIZE/2+1)+u]=height;
            text2[v*(TILE_SIZE/2+1)+u]=texture;
            shade2[v*(TILE_SIZE/2+1)+u]=shade_avg;
        }
    }
}

#define LOD_ADJUST 0.3f

void Tile::connectEdges(float *height, int where)
{
    int u,v;
    float h;

    if (where&EDGE_MINUS_V) {
        v=0;
        for (u=1;u<TILE_SIZE;u+=2) {
            h=height[v*(TILE_SIZE+1)+u-1];
            h+=height[v*(TILE_SIZE+1)+u+1];
            h/=2.0f;
            h+=LOD_ADJUST;
            height[v*(TILE_SIZE+1)+u]=h;
        }
    }

    if (where&EDGE_PLUS_V) {
        v=TILE_SIZE;
        for (u=1;u<TILE_SIZE;u+=2) {
            h=height[v*(TILE_SIZE+1)+u-1];
            h+=height[v*(TILE_SIZE+1)+u+1];
            h/=2.0f;
            h+=LOD_ADJUST;
            height[v*(TILE_SIZE+1)+u]=h;
        }
    }

    if (where&EDGE_MINUS_U) {
        u=0;
        for (v=1;v<TILE_SIZE;v+=2) {
            h=height[(v-1)*(TILE_SIZE+1)+u];
            h+=height[(v+1)*(TILE_SIZE+1)+u];
            h/=2.0f;
            h+=LOD_ADJUST;
            height[v*(TILE_SIZE+1)+u]=h;
        }
    }

    if (where&EDGE_PLUS_U) {
        u=TILE_SIZE;
        for (v=1;v<TILE_SIZE;v+=2) {
            h=height[(v-1)*(TILE_SIZE+1)+u];
            h+=height[(v+1)*(TILE_SIZE+1)+u];
            h/=2.0f;
            h+=LOD_ADJUST;
            height[v*(TILE_SIZE+1)+u]=h;
        }
    }
}

