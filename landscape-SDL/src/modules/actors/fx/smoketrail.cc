#include <cmath>
#include "smoketrail.h"
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <remap.h>

#define max(x,y) ((x)>(y)?(x):(y))

#define MIN_DIST                         10.0
#define MAX_DIST                         100.0
#define SMOKE_TRAIL_WIDTH                0.1
#define SMOKE_TRAIL_WIDTH_FINAL          6.0
#define MAX_AGE_IN_SECS                  3.0
#define TEXTURE_PER_SEC               1500.0
#define TEXTURE_PER_METER                0.015
#define COS_OF_MAX_ANGLE                 0.995
#define LERP(u,v,t,a,b) ((u)+((v)-(u))*(((t)-(a))/((b)-(a))))

SmokeTrail::SmokeTrail(Ptr<IGame> thegame)
    : SimpleActor(thegame)
{
    this->renderer = thegame->getRenderer();
    this->state=ALIVE;
    this->camera = thegame->getCamera();
    Ptr<IConfig> config( thegame->getConfig() );
    this->smoke = thegame->getTexMan()->query(
            config->query("SmokeTrail_smoke_tex"), JR_HINT_GREYSCALE);
    this->puffy = thegame->getTexMan()->query(
            config->query("SmokeTrail_puffy_tex"), JR_HINT_GREYSCALE);
    this->debug_mode = false;
    thegame->getEventRemapper()->map("debug",
            SigC::slot(*this, &SmokeTrail::toggleDebugMode));
}

void SmokeTrail::action()
{
    double delta_t = thegame->getTimeDelta() / 1000.0;

    // increase age for every trail point
    for(std::deque<TrailPoint>::iterator i=trail.begin(); i!=trail.end(); i++) {
        i->age += delta_t;
    }

    Vector p = pos->getLocation();
    setLocation( p );

    if ( IActor::ALIVE == pos->getState() ) {
        TrailPoint t;
        float dist;
        t.p = p;
        t.age = 0.0;
        if(trail.empty()) {
            t.tex_v = 0.0;
            dist = MIN_DIST;
        } else {
            if (trail.size() >= 2) {
                dist = (trail[1].p - t.p).length();
                t.tex_v = trail[1].tex_v + TEXTURE_PER_METER * dist;
            } else {
                dist = (trail[0].p - t.p).length();
                t.tex_v = trail[0].tex_v + TEXTURE_PER_METER * dist;
            }
        }

        if (dist < MIN_DIST && trail.size()>=2) {
            trail[0] = t;
            return;
        }

        if (dist <= MAX_DIST && trail.size() >= 3) {
            int s = trail.size();
            Vector p2(trail[0].p);
            Vector p3(trail[1].p);
            Vector p4(trail[2].p);
            float scalar=(t.p - p2).normalize() * (p3 - p4).normalize();
            if (scalar < COS_OF_MAX_ANGLE) {
                trail.push_front(t);
            } else {
                trail[0] = t;
            }
        } else {
            trail.push_front(t);
        }

    } else {
        // If last added trail point has become older than MAX_AGE_IN_SECS
        // we can die
        if ( trail.front().age > MAX_AGE_IN_SECS) {
            state = IActor::DEAD;
        }
    }
    if (!trail.empty()) {
        if (trail.size()==1) {
            if (trail.back().age > MAX_AGE_IN_SECS) {
                trail.pop_back();
            }
        } else while (trail[trail.size()-1].age > MAX_AGE_IN_SECS) {
            trail.pop_back();
        }
    }
}

void SmokeTrail::draw()
{
    if (trail.empty()) return;

    Vector eye = camera->getLocation();
    Vector v_right, v_up, v_front;
    camera->getOrientation(&v_right, &v_up, &v_front);

    Vector p;               // position vector
    Vector r;               // right vector
    Vector s;               // segment vector
    float  a;               // alpha value

    renderer->enableAlphaBlending();
    renderer->enableSmoothShading();
    renderer->enableTexturing();
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->setTexture(smoke);
    renderer->disableZBufferWriting();
    renderer->begin(JR_DRAWMODE_TRIANGLE_STRIP);
    renderer->setColor(Vector(1,1,1));

    typedef std::deque<TrailPoint>::iterator TrailIter;
    for (TrailIter i=trail.begin(); i!=trail.end(); i++) {

        p = i->p;
        if (i==trail.begin()) s = i[1].p - i[0].p;
        else if (i+1==trail.end()) s = i[0].p - i[-1].p;
        else s = (i[1].p - i[-1].p) / 2.0;

        r = (s % (p - eye)).normalize();
        r *= LERP(SMOKE_TRAIL_WIDTH, SMOKE_TRAIL_WIDTH_FINAL,
                i->age, 0.0, MAX_AGE_IN_SECS);

        a = LERP(1.0, 0.0, i->age, 0.0, MAX_AGE_IN_SECS);

        renderer->setAlpha(a);

        renderer->setUVW(Vector(0, -i->tex_v, 0));
        renderer->vertex(p - r);
        renderer->setUVW(Vector(1, -i->tex_v, 0));
        renderer->vertex(p + r);

    }

    renderer->end();
    renderer->enableZBufferWriting();
    renderer->disableAlphaBlending();
}

void SmokeTrail::follow(Ptr<IActor> pos)
{
    this->pos=pos;
}

void SmokeTrail::toggleDebugMode()
{
    debug_mode = !debug_mode;
}
