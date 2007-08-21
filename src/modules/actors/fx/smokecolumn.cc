#include "smokecolumn.h"
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <modules/clock/clock.h>

#define max(x,y) ((x)>(y)?(x):(y))

// A random value between 0.0 and 1.0
#define RAND ((float) rand() / (float) RAND_MAX)
// A random value between -1.0 and 1.0
#define RAND2 (-1.0 + 2.0 * ((float) rand() / (float) RAND_MAX))
#define PI 3.141593
#define RANDVEC Vector(RAND2, RAND2, RAND2)

SmokeColumn::SmokePuff::SmokePuff(
        const Vector &p,
        float opacity,
        const PuffParams & params)
{
    this->v = params.direction_vector + params.direction_deviation * RANDVEC;
    this->p = p + params.pos_deviation * RANDVEC;
    this->ttl = params.ttl.a + params.ttl.length() * RAND2;
    this->age = 0;
    this->omega = params.omega.a + params.omega.length() * RAND2;
    this->opacity = opacity;
}

void SmokeColumn::SmokePuff::action(IGame *game,
        double time_passed,
        const PuffParams & params)
{
    p += v * time_passed;
    v += (params.wind_vector - v) * params.wind_influence * time_passed;
    age += time_passed;
}

void SmokeColumn::SmokePuff::draw(JRenderer *r,
        const Vector &right, const Vector &up, const Vector &front,
        const PuffParams & params)
{
    float size = params.start_size + (age / ttl) *
            (params.end_size - params.start_size);
    Matrix3 M2 = MatrixFromColumns<float>(right, up, front)
                * RotateZMatrix<float>(omega * age);
    Matrix M = TranslateMatrix<4,float>(p)
            * Matrix::Hom(M2);

    float alpha;
    if (age < 0.3) alpha = age/0.3;
    else alpha = 1.0 - (age-0.3)/(ttl-0.3);
    alpha *= opacity;
    
    r->setAlpha(alpha);
    r->setColor(params.color);
    r->pushMatrix();
    r->multMatrix(M);
    r->begin(JR_DRAWMODE_TRIANGLE_FAN);
    
    r->setUVW(Vector(1,1,0)); *r << size*Vector(1,1,0);
    r->setUVW(Vector(1,0,0)); *r << size*Vector(1,-1,0);
    r->setUVW(Vector(0,0,0)); *r << size*Vector(-1,-1,0);
    r->setUVW(Vector(0,1,0)); *r << size*Vector(-1,1,0);
    
    r->end();
    r->popMatrix();
}


SmokeColumn::SmokeColumn(Ptr<IGame> thegame, const Vector &pos,
        const Params & params, const PuffParams & puff_params)
    : SimpleActor(thegame),
      params(params), puff_params(puff_params),
      age(0.0), next_puff(0.0)
{
    setLocation(pos);
    this->renderer = thegame->getRenderer();
    Ptr<IConfig> config( thegame->getConfig() );
    this->smoke_tex = thegame->getTexMan()->query(
            config->query("SmokeColumn_puffy_tex"), JR_HINT_GREYSCALE);
}

void SmokeColumn::action()
{
    double time_delta = thegame->getClock()->getStepDelta();
    age +=time_delta;
    if (age > params.ttl && smokelist.size()==0) { state = DEAD; return; }

    if (age < params.ttl) {
        next_puff += time_delta;
        //ls_message("next_puff = %e\n", next_puff);
        while ( next_puff > params.interval) {
            next_puff -= params.interval;
            smokelist.insert(smokelist.end(),
                    new SmokePuff(getLocation(),
                                  1.0 - pow(age/params.ttl, 5.0),
                                  puff_params));
        }
    }

    for (SmokeIterator i=smokelist.begin(); i!=smokelist.end(); i++) {
        (*i)->action(&*thegame, time_delta, puff_params);
        if ((*i)->isDead()) {
            delete *i;
            smokelist.erase(i++);
        }
    }
}

void SmokeColumn::draw()
{
    Ptr<ICamera> camera = thegame->getCamera();
    Vector eye = camera->getLocation();
    Vector up, right, front;
    camera->getOrientation(&up, &right, &front);

    renderer->enableAlphaBlending();
    renderer->setVertexMode(JR_VERTEXMODE_GOURAUD_TEXTURE);
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->setTexture(smoke_tex->getTxtid());
    renderer->disableZBufferWriting();

    renderer->begin(JR_DRAWMODE_TRIANGLES);
    for (SmokeIterator i=smokelist.begin(); i!=smokelist.end(); i++) {
        (*i)->draw(renderer, right, up, front, puff_params);
        //thegame->drawDebugTriangleAt((*i)->p);
    }
    renderer->end();

    renderer->enableZBufferWriting();
    renderer->disableAlphaBlending();
}


FollowingSmokeColumn::FollowingSmokeColumn(Ptr<IGame> thegame,
        const Params & params, const PuffParams & puff_params)
        : SmokeColumn(thegame, Vector(0,0,0), params, puff_params)
{ };

void FollowingSmokeColumn::follow(Ptr<IActor> t)
{
    target=t;
    setLocation(target->getLocation());
}


#define INTERP(a, b, t, t0, t1) ((a) + ((b)-(a)) * ((t) - (t0)) / ((t1) - (t0)))
void FollowingSmokeColumn::action()
{
    double time_delta = thegame->getClock()->getStepDelta();
    // Save old position for interpolation
    Vector p0 = getLocation();

    age +=time_delta;
    if (age > params.ttl && smokelist.size()==0) { state = DEAD; return; }

    if(target->getState() == ALIVE) {
        setLocation(target->getLocation());
        Vector p = getLocation();

        next_puff += time_delta;
        //ls_message("next_puff = %e\n", next_puff);
        while ( next_puff > params.interval) {
            next_puff -= params.interval;
            SmokePuff *puff = new SmokePuff(
                    INTERP(p0, p, next_puff, 0.0, time_delta),
                    1.0,
                    puff_params);
            smokelist.insert(smokelist.end(), puff);
        }
    } else if (smokelist.empty()) {
        state = DEAD;
    }

    for (SmokeIterator i=smokelist.begin(); i!=smokelist.end(); i++) {
        (*i)->action(&*thegame, time_delta, puff_params);
        if ((*i)->isDead()) {
            delete *i;
            smokelist.erase(i++);
        }
    }
}
