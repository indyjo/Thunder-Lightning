#include <interfaces/IConfig.h>
#include <modules/engines/effectors.h>
#include <modules/engines/rigidengine.h>
#include <modules/model/modelman.h>
#include "dumbmissile.h"

// A dumb missile inspired by the Hydra 70 with data from
// http://www.fas.org/man/dod-101/sys/missile/hydra-70.htm

#define BLAST_BEGIN 0
#define BLAST_END 1.05
#define BLAST_THRUST 6007.0


DumbMissile::DumbMissile(Ptr<IGame> thegame, Ptr<IActor>, Ptr<IActor> source)
    : Missile(thegame, source)
{
    setTargetInfo(new TargetInfo("Dumb missile", 1.0f, TargetInfo::MISSILE));

    float mass = 4.54; 
    engine->construct(mass, 6, 6, 2);
    thrust->setMaxForce(Vector(0,0,BLAST_THRUST));
    
    blast_begin = BLAST_BEGIN;
    blast_end = BLAST_END;
    min_explosion_age = 0.0f;
    
    setModel(thegame->getModelMan()->query(
    	thegame->getConfig()->query("DumbMissile_model")));
}


