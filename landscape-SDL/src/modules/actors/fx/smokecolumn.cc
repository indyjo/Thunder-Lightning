#include "smokecolumn.h"
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>

#define max(x,y) ((x)>(y)?(x):(y))

// A random value between 0.0 and 1.0
#define RAND ((float) rand() / (float) RAND_MAX)
// A random value between -1.0 and 1.0
#define RAND2 (-1.0 + 2.0 * ((float) rand() / (float) RAND_MAX))
#define PI 3.141593

SmokeColumn::SmokePuff::SmokePuff(
        const Vector &p,
        float opacity,
        const PuffParams & params)
{
    this->dead = false;
    this->v = Vector(params.direction_vector) + params.direction_deviation
            * Vector(RAND2, RAND2, RAND2);
    this->p = p + params.pos_deviation * Vector(RAND2, RAND2, RAND2);
    this->ttl = params.ttl + params.ttl_deviation * RAND2;
    this->age = 0.0;
    this->opacity = opacity;
}

void SmokeColumn::SmokePuff::action(IGame *game,
        double time_passed,
        const PuffParams & params)
{
    p += v * time_passed;
    v += (Vector(params.wind_vector) - v) * params.wind_influence * time_passed;
    age += time_passed;
    if (age > ttl) dead = true;
}

void SmokeColumn::SmokePuff::draw(JRenderer *r,
        const Vector &right, const Vector &up, const Vector &front,
        const PuffParams & params)
{
    jvertex_coltxt puffy_v[4] = {
        {{ 0, 0, 0}, {255, 255, 255}, { 0,  0,  0}},
        {{ 0, 0, 0}, {255, 255, 255}, {64,  0,  0}},
        {{ 0, 0, 0}, {255, 255, 255}, {64, 64,  0}},
        {{ 0, 0, 0}, {255, 255, 255}, { 0, 64,  0}}
    };
    Vector v[2][2];
    float size = params.start_size + (age / ttl) *
            (params.end_size - params.start_size);
    Matrix3 M2 = MatrixFromColumns<float>(right, up, front)
                * RotateZMatrix<float>(params.rotations_in_life * age/ttl * PI);
    Matrix M = TranslateMatrix<4,float>(p)
            * Matrix::Hom(M2);
    for (int i=0; i<2; i++) {
        for (int j=0; j<2; j++) {
            v[i][j]  = M * Vector(
                    ((float) i - 0.5) * size,
                    ((float) j - 0.5) * size,
                    0.0);
        }
    }

    for (int i=0; i<4; i++) {
        puffy_v[i].col.r = params.color[0];
        puffy_v[i].col.g = params.color[1];
        puffy_v[i].col.b = params.color[2];
    }

    puffy_v[0].p.x = v[0][0][0];
    puffy_v[0].p.y = v[0][0][1];
    puffy_v[0].p.z = v[0][0][2];

    puffy_v[1].p.x = v[1][0][0];
    puffy_v[1].p.y = v[1][0][1];
    puffy_v[1].p.z = v[1][0][2];

    puffy_v[2].p.x = v[1][1][0];
    puffy_v[2].p.y = v[1][1][1];
    puffy_v[2].p.z = v[1][1][2];

    puffy_v[3].p.x = v[0][1][0];
    puffy_v[3].p.y = v[0][1][1];
    puffy_v[3].p.z = v[0][1][2];

    float alpha;
    if (age < 0.3) alpha = age/0.3;
    else alpha = 1.0 - (age-0.3)/(ttl-0.3);
    alpha *= opacity;
    r->setAlpha(alpha);
    r->addVertex(&puffy_v[0]);
    r->addVertex(&puffy_v[1]);
    r->addVertex(&puffy_v[2]);
    r->addVertex(&puffy_v[2]);
    r->addVertex(&puffy_v[3]);
    r->addVertex(&puffy_v[0]);
}


SmokeColumn::SmokeColumn(Ptr<IGame> thegame, const Vector &pos,
        const Params & params, const PuffParams & puff_params)
    : SimpleActor(thegame),
      params(params), puff_params(puff_params),
      age(0.0), next_puff(0.0)
{
    setLocation(pos);
    this->renderer = thegame->getRenderer();
    this->camera = thegame->getCamera();
    Ptr<IConfig> config( thegame->getConfig() );
    this->smoke_tex = thegame->getTexMan()->query(
            config->query("SmokeColumn_puffy_tex"), JR_HINT_GREYSCALE);
}

void SmokeColumn::action()
{
    double time_delta = thegame->getTimeDelta() / 1000.0;
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
    Vector eye = camera->getLocation();
    Vector up, right, front;
    camera->getOrientation(&up, &right, &front);

    renderer->enableAlphaBlending();
    renderer->setVertexMode(JR_VERTEXMODE_GOURAUD_TEXTURE);
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->setTexture(smoke_tex);
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
    double time_delta = thegame->getTimeDelta() / 1000.0;
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
    } else {
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
