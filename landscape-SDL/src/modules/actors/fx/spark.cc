#include "spark.h"

#define EARTH_GRAVITY 9.81
#define MAX_DEVIATION_PER_SECOND 1.0
#define MIN_LIFETIME 1.0
#define MAX_LIFETIME 2.0

Spark::Spark(Ptr<IGame> thegame)
    : SimpleActor(thegame)
{
    this->renderer = thegame->getRenderer();
    this->state=ALIVE;
    this->terrain = thegame->getTerrain();
}

void Spark::action()
{
    float random_deviation[3];
    double time_in_secs = thegame->getTimeDelta() / 1000.0;
    
    if ((lifetime_left-=time_in_secs) < 0.0) state=DEAD;
    
    SimpleActor::action();
    
    Vector p = getLocation();
    
    TrailPoint t;
    t.p[0] = p[0];
    t.p[1] = p[1];
    t.p[2] = p[2];
    trail.add(t);
}

void Spark::draw()
{
    jvertex_col v1={{ 0.0f, 0.0f, 0.0f},{255.0f,185.0f,100.0f}};

    if (trail.getSize() > 1) {
        renderer->enableAlphaBlending();
        renderer->setBlendMode(JR_BLENDMODE_ADDITIVE);
        renderer->disableZBufferWriting();
        renderer->setVertexMode(JR_VERTEXMODE_GOURAUD);
        
        // Draw a point so the spark won't disappear on the screen
        renderer->begin(JR_DRAWMODE_POINTS);
        Vector tp = trail[trail.getSize()-1].p;
        v1.p.x = tp[0];
        v1.p.y = tp[1];
        v1.p.z = tp[2];
        renderer->setAlpha(sqrt(lifetime_left / MIN_LIFETIME));
        renderer->addVertex(&v1);
        renderer->end();
        
        renderer->begin(JR_DRAWMODE_CONNECTED_LINES);
        for (int i=0, t_len=trail.getSize(); i<t_len; i++) {
            v1.p.x = trail[i].p[0];
            v1.p.y = trail[i].p[1];
            v1.p.z = trail[i].p[2];
            float alpha = (float) (i + SPARK_TRAIL_BUFFER - t_len) / (float) SPARK_TRAIL_BUFFER;
            alpha *= sqrt(lifetime_left / MIN_LIFETIME);
            renderer->setAlpha(alpha);
            renderer->addVertex(&v1);
        }
        renderer->end();
        renderer->enableZBufferWriting();
        renderer->setBlendMode(JR_BLENDMODE_BLEND);
        renderer->disableAlphaBlending();
    }
}

#define RAND_POS ((float) rand() / (float) RAND_MAX)
void Spark::shoot(const Vector &pos, const Vector &vec, const Vector &dir)
{
    setLocation(pos);
    setMovementVector(vec);
    TrailPoint t;
    t.p[0] = pos[0];
    t.p[1] = pos[1];
    t.p[2] = pos[2];
    trail.add(t);
    lifetime_left = MIN_LIFETIME + RAND_POS * (MAX_LIFETIME - MIN_LIFETIME);
}
