#ifndef SMOKECOLUMN_H
#define SMOKECOLUMN_H

#include <landscape.h>
#include <modules/actors/simpleactor.h>
#include <cstdio>
#include <cmath>
#include <list>


class SmokeColumn: public SimpleActor
{
public:
    struct Params
    {
        Params() : interval(1), ttl(40) { };
        double interval, ttl;
    };

    struct PuffParams
    {
        PuffParams() 
                : ttl(40), ttl_deviation(20),
                pos_deviation(3),direction_deviation(2),
                start_size(15), end_size(200),
                rotations_in_life(15),
                direction_vector(0,10,0),
                wind_vector(10,4,0), wind_influence(0.1),
                color(200, 200, 200)
        { }
        double ttl, ttl_deviation;
        float pos_deviation;
        float direction_deviation;
        float start_size, end_size;
        float rotations_in_life;
        Vector direction_vector;
        Vector wind_vector;
        float wind_influence;
        Vector color;
    };
    
    class SmokePuff {
        friend class SmokeColumn;
        
        Vector p, v;    // the puff coords and direction
        double age, ttl; // time to live
        float opacity;
        bool dead;
        
    public:
        SmokePuff(const Vector &p, float opacity, const PuffParams & params);
        inline bool isDead() { return dead; };
        void action(IGame *game, double time_passed, const PuffParams & params);
        void draw(JRenderer *r,
                const Vector &right, const Vector &up, const Vector &front,
                const PuffParams & params);
    };

    typedef std::list<SmokePuff*> SmokeList;
    typedef SmokeList::iterator SmokeIterator;
    
public:
    SmokeColumn(Ptr<IGame> thegame, const Vector &pos,
        const Params     & params = Params(),
        const PuffParams & puff_params = PuffParams());

    virtual void action();

    virtual void draw();
    
protected:
    double age;
    double next_puff;
    Params params;
    PuffParams puff_params;
    JRenderer *renderer;
    Ptr<ICamera> camera;
    SmokeList smokelist;
    TexPtr smoke_tex;
};

class FollowingSmokeColumn: public SmokeColumn, public IFollower
{
    Ptr<IActor> target;
public:
    FollowingSmokeColumn(Ptr<IGame> thegame,
        const Params & params = Params(),
        const PuffParams & puff_params = PuffParams());
    
    virtual void follow(Ptr<IActor>);
    virtual void action();
};


#endif
