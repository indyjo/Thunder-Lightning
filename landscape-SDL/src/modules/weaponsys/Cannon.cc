#include <cstdlib>
#include <interfaces/IConfig.h>
#include <modules/actors/projectiles/bullet.h>
#include <sound.h>
#include "Cannon.h"
#include "Armament.h"

#define RAND2 ((float) rand() / (float) RAND_MAX * 2.0 - 1.0)

Cannon::Cannon(Ptr<IGame> game, const std::string & name, int rounds, float loadtime, bool singleshot)
: Weapon(name,rounds,loadtime, singleshot), game(game)
{ }

void Cannon::onFire(Armament & arm) {
    Ptr<Bullet> projectile( new Bullet(ptr(game), arm.getSourceActor()) );
    projectile->setTTL(5);
    projectile->setNoCollideParent(arm.getNoCollideParent());

    Ptr<IPositionProvider> & position = barrels[next_barrel].position;
    Vector start = position->getLocation();
    Vector move = arm.getSourceActor()->getMovementVector();
    Vector front = position->getFrontVector();
    Vector right = position->getRightVector();
    Vector up = position->getUpVector();

    Ptr<SoundSource> snd_src = game->getSoundMan()->requestSource();
    snd_src->setPosition(start);
    snd_src->setVelocity(move);
    snd_src->play(game->getSoundMan()->querySound(
            game->getConfig()->query("Cannon_sound")));
    game->getSoundMan()->manage(snd_src);

    Vector deviation(0,0,0);
    for(int i=0; i<5; ++i)
    	deviation += Vector(RAND2,RAND2,0);
    deviation *= 0.001/5;
    
    float muzzle_velocity = game->getConfig()->queryFloat("Cannon_muzzle_velocity", 1200);
    move += muzzle_velocity * (front + right*deviation[0] + up*deviation[1]);

    game->addActor(projectile);
    //ls_message("Shooting bullet from: ");start.dump();
    //ls_message("to: "); move.dump();
    projectile->shoot(start, move, front);
}
