#include <tnl.h>

#include "smokecolumn.h"
#include "explosion.h"
#include "smoketrail.h"
#include "spark.h"

#include "SpecialEffects.h"

#define RAND ((float) rand() / (float) RAND_MAX)
#define RAND2 ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)

#define MAX_EXPLOSION_SIZE 4.0
#define MIN_EXPLOSION_SIZE 1.0
#define MAX_EXPLOSION_DISTANCE 15.0
#define NUM_EXPLOSIONS 15
#define MAX_EXPLOSION_AGE -2.0

void aircraftFirstExplosion(Ptr<IGame> game, Ptr<IActor> aircraft) {
    {
        SmokeColumn::Params params;
        params.interval=0.01;
        params.ttl=1.2;
        SmokeColumn::PuffParams puffparams;
        puffparams.start_size=0.5f;
        puffparams.end_size=1.5f;
        puffparams.color=Vector(0.4,0.4,0.4);
        puffparams.ttl=Interval(3,5);
        puffparams.pos_deviation=0.2;
        puffparams.direction_deviation=0.5;
        Ptr<FollowingSmokeColumn> smoke =
               new FollowingSmokeColumn(game, params, puffparams);
        smoke->follow(aircraft);
        game->addActor(smoke);
        
        Vector p = aircraft->getLocation();
        for(int i=0; i<5; i++) {
            Vector v = p + RAND * MAX_EXPLOSION_DISTANCE *
                    Vector(RAND2, RAND2, RAND2).normalize();
            float size = 1 + RAND * 2;
            double time = RAND * MAX_EXPLOSION_AGE;
            Ptr<Explosion> explosion = new Explosion(game, v, size, time);
            explosion->setMovementVector((0.1 + 0.9*RAND)*aircraft->getMovementVector());
            game->addActor(explosion);
        }
    }
}


void aircraftFinalExplosion(Ptr<IGame> game, Ptr<IActor> aircraft) {
    {
        Vector p = aircraft->getLocation();
        for(int i=0; i<25; i++) {
            Vector v = p + RAND * MAX_EXPLOSION_DISTANCE *
                    Vector(RAND2, RAND2, RAND2).normalize();
            float size = MIN_EXPLOSION_SIZE +
                    RAND * (MAX_EXPLOSION_SIZE - MIN_EXPLOSION_SIZE);
            double time = RAND * MAX_EXPLOSION_AGE;
            Ptr<Explosion> explosion = new Explosion(game, v, size, time);
            explosion->setMovementVector((0.1 + 0.9*RAND)*aircraft->getMovementVector());
            game->addActor(explosion);
        }
    }
    
    {
        SmokeColumn::Params params;
        SmokeColumn::PuffParams puff_params;
        
        // Setup smoke column parameters so that the puff interval is shorter
        params.ttl = 1.0;
        params.interval = 0.01;
        
        // Setup smoke puff parameters so that their time to live is shorter
        // and they are much smaller then the default
        puff_params.ttl = Interval(1,3);
        puff_params.pos_deviation = 0.5;
        puff_params.start_size = 0.5;
        puff_params.end_size = 2.0;
        
        for (int i=0; i<20; i++) {
            Ptr<Spark> spark(new Spark(game));
            Vector vec = aircraft->getMovementVector() + 60*Vector(RAND2,RAND2,RAND2);
            spark->shoot(aircraft->getLocation(), vec, Vector(vec).normalize());
            game->addActor(spark);
            Ptr<FollowingSmokeColumn> smoke = 
                    new FollowingSmokeColumn(game, params, puff_params);
            smoke->follow(spark);
            game->addActor(smoke);
        }
        for (int i=0; i<180; i++) {
            Ptr<Spark> spark(new Spark(game));
            Vector vec = aircraft->getMovementVector() + 60*Vector(RAND2,RAND2,RAND2);
            spark->shoot(aircraft->getLocation(), vec, Vector(vec).normalize());
            game->addActor(spark);
        }
    }

}


void tankFirstExplosion(Ptr<IGame> game, Ptr<IActor> tank) {
    aircraftFirstExplosion(game, tank);
}

void tankFinalExplosion(Ptr<IGame> game, Ptr<IActor> tank) {
    aircraftFinalExplosion(game, tank);
}

