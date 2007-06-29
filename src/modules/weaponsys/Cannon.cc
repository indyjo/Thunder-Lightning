#include <cstdlib>
#include <interfaces/IConfig.h>
#include <modules/actors/projectiles/bullet.h>
#include <sound.h>
#include "Cannon.h"
#include "Armament.h"

#define RAND2 ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)

Cannon::Cannon(Ptr<IGame> game, const std::string & name, int rounds, float loadtime, bool singleshot)
: Weapon(name,rounds,loadtime, singleshot), game(game), factor(1.0f)
{ }

void Cannon::onFire() {
    Ptr<Bullet> projectile( new Bullet(ptr(game), armament->getSourceActor(), factor) );
    projectile->setTTL(5);
    projectile->setNoCollideParent(armament->getNoCollideParent());

    Ptr<IPositionProvider> & position = barrels[next_barrel].position;
    Vector start = position->getLocation();
    Vector move = armament->getSourceActor()->getMovementVector();
    Vector front = position->getFrontVector();
    Vector right = position->getRightVector();
    Vector up = position->getUpVector();

    Ptr<IConfig> cfg = game->getConfig();
    
    Ptr<SoundSource> snd_src = game->getSoundMan()->requestSource();
    snd_src->setPosition(start);
    snd_src->setVelocity(move);
    snd_src->setGain(cfg->queryFloat(name+"_gain", 1.0f));
    snd_src->setReferenceDistance(cfg->queryFloat(name+"_reference_distance", 1.0f));
    snd_src->play(game->getSoundMan()->querySound( cfg->query(name+"_sound")));
    game->getSoundMan()->manage(snd_src);

    Vector deviation(0,0,0);
    for(int i=0; i<5; ++i)
    	deviation += Vector(RAND2,RAND2,0);
    deviation *= 0.001/5;
    
    float muzzle_velocity = cfg->queryFloat(name+"_muzzle_velocity", 1200);
    move += muzzle_velocity * (front + right*deviation[0] + up*deviation[1]);

    game->addActor(projectile);
    //ls_message("Shooting bullet from: ");start.dump();
    //ls_message("to: "); move.dump();
    projectile->shoot(start, move, front);
}
