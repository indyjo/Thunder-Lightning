#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include "explosion.h"
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <modules/jogi/jogi.h>
#include <modules/drawing/billboard.h>
#include <sound.h>


#define RAND ((float) rand() / (float) RAND_MAX)
#define RAND2 (-1.0 + 2.0 * ((float) rand() / (float) RAND_MAX))
#define PI 3.141593

namespace {
    bool file_exists(const char *name) { std::ifstream in(name); return in; }
}

Explosion::Explosion(Ptr<IGame> thegame, const Vector & pos,
        float size_factor,
        double init_age,
        bool with_sound)
: SimpleActor(thegame), age(init_age),
  renderer(thegame->getRenderer()), size_factor(size_factor)
{
    setLocation(pos);

    Ptr<IConfig> cfg = thegame->getConfig();
    frames=atoi( cfg->query("Explosion_frames","0") );

    tex = new TexPtr[frames];

    secs_per_frame=atof( cfg->query("Explosion_seconds_per_frame","0.041") );
    size=0.5 * atof( cfg->query("Explosion_size","20.0") );
    const char * prefix=cfg->query("Explosion_tex_prefix");
    const char * postfix=cfg->query("Explosion_tex_postfix",".spr");
    for(int i=0; i<frames; i++) {
        char buffer[256];
        snprintf(buffer, 256, "%s%04d%s", prefix, i, postfix);
        if (!file_exists(buffer))
            ls_warning("[Explosion]: Cannot open texture file %s\n", buffer);
        tex[i] = thegame->getTexMan()->query(buffer, JR_HINT_FULLOPACITY);
    }

    rot = RAND * 2.0 * PI;
    rot_speed = (RAND2 * PI) / size_factor;

    
    if (with_sound) {
        Ptr<SoundMan> soundman = thegame->getSoundMan();
        Ptr<SoundSource> soundsource = soundman->requestSource();
        if (soundsource) {
            int sndidx = int(RAND*cfg->queryInt("Explosion_num_sounds", 1));
            std::ostringstream sndname;
            sndname << cfg->query("Explosion_sound_prefix");
            sndname << (sndidx+1);
            sndname << cfg->query("Explosion_sound_postfix");
            
            Ptr<Sound> snd = soundman->querySound(sndname.str());
            
            soundsource->play( snd );
            //soundsource->play( new Sound("/home/jonas/devel/gcc/landscape-SDL/install/share/"
            //        "landscape/sounds/explosion-01.wav"));
            soundsource->setGain(1);
            soundsource->setReferenceDistance(15*size_factor);
            soundsource->setPosition(getLocation());
            soundman->manage(soundsource);
        }
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

    Ptr<ICamera> camera = thegame->getCamera();

    renderer->enableAlphaBlending();
    renderer->setBlendMode(JR_BLENDMODE_ADDITIVE);
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->setTexture(tex->getTxtid());
    renderer->enableTexturing();
    
    renderer->disableZBufferWriting();
    renderer->disableFog();
    
    renderer->setColor(Vector(1,1,1));
    renderer->setAlpha(1.0);
    drawBillboard(renderer, getLocation(), camera, rot, size*size_factor, size*size_factor);

    renderer->enableFog();
    renderer->enableZBufferWriting();
    renderer->setBlendMode(JR_BLENDMODE_BLEND);
    renderer->disableAlphaBlending();
    renderer->disableTexturing();
}

