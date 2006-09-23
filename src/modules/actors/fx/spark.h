#ifndef SPARK_H
#define SPARK_H

#include <cmath>
#include <landscape.h>
#include <util.h>
#include <modules/actors/simpleactor.h>
#include <interfaces/IProjectile.h>

#define SPARK_TRAIL_BUFFER 30

class Spark: public SimpleActor
{
    typedef struct {
        float p[3];    // the point coords
    } TrailPoint;

public:
    Spark(Ptr<IGame> thegame);

    virtual void action();
    
    virtual void draw();
    
    virtual void shoot(const Vector &pos, const Vector &vec, const Vector &dir);
    
private:
    double lifetime_left;
    JRenderer *renderer;
    Ptr<ITerrain> terrain;
    buffer<TrailPoint, SPARK_TRAIL_BUFFER> trail;
};

#endif
