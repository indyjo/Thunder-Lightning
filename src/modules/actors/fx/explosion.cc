#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include "explosion.h"
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <modules/jogi/jogi.h>
#include <sound.h>


#define RAND ((float) rand() / (float) RAND_MAX)
#define RAND2 (-1.0 + 2.0 * ((float) rand() / (float) RAND_MAX))
#define PI 3.141593

namespace {
    bool file_exists(const char *name) { std::ifstream in(name); return in; }
}

Explosion::Explosion(Ptr<IGame> thegame, const Vector & pos,
        float size_factor,
        double init_age)
: SimpleActor(thegame), age(init_age),
  renderer(thegame->getRenderer()), size_factor(size_factor)
{
    setLocation(pos);

    camera = thegame->getCamera();

    Ptr<IConfig> cfg = thegame->getConfig();
    frames=atoi( cfg->query("Explosion_frames","0") );

    tex = new TexPtr[frames];

    secs_per_frame=atof( cfg->query("Explosion_seconds_per_frame","0.041") );
    size=0.5 * atof( cfg->query("Explosion_size","20.0") );
    const char * prefix=cfg->query("Explosion_filename_prefix");
    const char * postfix=cfg->query("Explosion_filename_postfix",".spr");
    for(int i=0; i<frames; i++) {
        char buffer[256];
        snprintf(buffer, 256, "%s%04d%s", prefix, i, postfix);
        if (!file_exists(buffer))
            ls_warning("[Explosion]: Cannot open texture file %s\n", buffer);
        tex[i] = thegame->getTexMan()->query(buffer, JR_HINT_FULLOPACITY);
    }

    rot = RAND * 2.0 * PI;
    rot_speed = (RAND2 * PI) / size_factor;

    
    Ptr<SoundMan> soundman = thegame->getSoundMan();
    Ptr<SoundSource> soundsource = soundman->requestSource();
    if (soundsource) {
        soundsource->play( soundman->querySound("explosion-01.wav") );
        //soundsource->play( new Sound("/home/jonas/devel/gcc/landscape-SDL/install/share/"
        //        "landscape/sounds/explosion-01.wav"));
        soundsource->setGain(1);
        soundsource->setReferenceDistance(15*size_factor);
        soundsource->setPosition(getLocation());
        soundman->manage(soundsource);
    }
}

Explosion::~Explosion() {
    delete [] tex;
}

void Explosion::action() {
    double delta_t = thegame->getTimeDelta() / 1000.0;
    age+=delta_t;
    rot += rot_speed * delta_t;
    //SimpleActor::action();
}

IActor::State Explosion::getState() {
    return (age>=secs_per_frame*(frames-1))?DEAD:ALIVE;
}

void Explosion::draw() {
    if (age < 0) return;
    int framenum = (int) (age / secs_per_frame);
    if (framenum >= frames) {
        //ls_error("[Explosion]: Sorry, but I have no texture #%d\n", framenum);
        return;
    }

    TexPtr tex=this->tex[framenum];

    Vector cam = camera->getLocation();
    Vector up = camera->getUpVector();
    Vector right = camera->getRightVector();
    Vector dist(getLocation()-cam);
    Vector sprite_pos(cam + 0.9 * dist);
    dist.normalize();
    Matrix R = Matrix::Hom(RotateAxisMatrix(dist, (float)rot));

    static const float x_coord[5] = {-1, 1, 1, -1, -1};
    static const float y_coord[5] = {1, 1, -1, -1, 1};
    static const float u_coord[5] = {0, 1, 1, 0, 0};
    static const float v_coord[5] = {0, 0, 1, 1, 0};
    jvertex_coltxt vtx={{0,0,0},{255,255,255},{0,0,0}};
    //jvertex_coltxt vtx={{0,0,0},{0,0,0},{0,0,0}};

    renderer->enableAlphaBlending();
    renderer->setBlendMode(JR_BLENDMODE_ADDITIVE);
    renderer->setVertexMode(JR_VERTEXMODE_GOURAUD_TEXTURE);
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->setTexture(tex);
    renderer->disableZBufferWriting();
    renderer->disableFog();
    renderer->begin(JR_DRAWMODE_TRIANGLE_FAN);
    renderer->setAlpha(0.5);
    for (int i=0; i<5; i++) {
        Vector v = (size*size_factor) * (x_coord[i]*right + y_coord[i]*up);
        v = R * v;
        v += sprite_pos;
        vtx.p.x=v[0];
        vtx.p.y=v[1];
        vtx.p.z=v[2];

        vtx.txt.x = (float) (tex.getWidth()-1)  * u_coord[i];
        vtx.txt.y = (float) (tex.getHeight()-1) * v_coord[i];

        renderer->addVertex(&vtx);
    }
    renderer->end();
    renderer->enableFog();
    renderer->enableZBufferWriting();
    renderer->setBlendMode(JR_BLENDMODE_BLEND);
    renderer->disableAlphaBlending();
}
