#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <tnl.h>
#include <modules/actors/simpleactor.h>
#include <modules/texman/TextureManager.h>

class Explosion: public SimpleActor
{
public:
    Explosion(Ptr<IGame> thegame, const Vector & pos,
        float size_factor=1.0,
        double init_age=0.0);
    virtual ~Explosion();

    virtual void action();
    virtual State getState();

    virtual void draw();
    
private:
    double age;
    JRenderer *renderer;
    Ptr<IPositionProvider> camera;
    TexPtr *tex;
    int frames;
    double secs_per_frame;
    float size, size_factor;
    double rot, rot_speed;
};

#endif
