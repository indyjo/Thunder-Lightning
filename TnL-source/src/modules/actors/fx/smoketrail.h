#ifndef SMOKETRAIL_H
#define SMOKETRAIL_H

#include <deque>
#include <tnl.h>
#include <modules/actors/simpleactor.h>
#include <modules/texman/TextureManager.h>
#include <interfaces/IFollower.h>

#define SMOKE_TRAIL_BUFFER 200

class SmokeTrail: public SimpleActor,
                  virtual public IFollower,
                  virtual public SigObject
{
    typedef struct {
        Vector p;    // the point coords
        double age;
        float  tex_v; // texture v coordinate
    } TrailPoint;

public:
    SmokeTrail(Ptr<IGame> thegame);

    virtual void action();

    virtual void draw();
    
    virtual void follow(Ptr<IActor>);

private:
    void shootSparks();
    void toggleDebugMode();

private:
    double start_time;
    double life_time;
    Vector last_segment, last_point, last_solid_point;
    Ptr<IActor> pos;
    JRenderer *renderer;
    std::deque<TrailPoint> trail;
    TexPtr smoke, puffy;
    bool debug_mode;
};

#endif
