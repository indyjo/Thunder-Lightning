#include <modules/collide/Collidable.h>
#include <modules/scripting/IoScriptingManager.h>
#include <modules/scripting/mappings.h>
#include "Armament.h"
#include "ProjectileLauncher.h"

ProjectileLauncher::ProjectileLauncher(
    Ptr<IGame> game,
    Ptr<Targeter> targeter,
    IProjectileFactory * factory,
    const std::string & name,
    int rounds)
:     Weapon(game->getConfig(), name,rounds)
    , game(game)
    , targeter(targeter)
    , factory(factory)
{
    Ptr<IConfig> cfg = game->getConfig();
    launch_speed = cfg->queryFloat(name+"_launch_speed", 0.0f);
    needs_target = cfg->queryFloat(name+"_needs_target", true);
}


bool ProjectileLauncher::canFire() {
    if (needs_target && ptr(targeter->getCurrentTarget())==0) 
        return false;
    return Weapon::canFire();
}

WeakPtr<IActor> ProjectileLauncher::onFire() {
    Ptr<IProjectile> projectile = factory->create(game, targeter->getCurrentTarget(), armament->getSourceActor());
    
    Ptr<Collide::Collidable> collidable = projectile->asCollidable();
    if (collidable) {
        collidable->setNoCollideParent(armament->getNoCollideParent());
    }

    Ptr<IPositionProvider> & position = barrels[next_barrel].position;
    Vector start = position->getLocation();
    Vector move = armament->getSourceActor()->getMovementVector();
    Vector front = position->getFrontVector();
    Vector right = position->getRightVector();
    Vector up = position->getUpVector();

    move += launch_speed * front;

    game->addActor(projectile);
    //ls_message("Shooting bullet from: ");start.dump();
    //ls_message("to: "); move.dump();
    projectile->shoot(start, move, front);
    
    // Send the target a message signalling the missile shot
    IoObject *self = IOCLONE(game->getIoScriptingManager()->getMainState()->objectProto);
    IoState_pushRetainPool(IOSTATE);
    IoState_stackRetain_(IOSTATE,self);

    IoObject_setSlot_to_(self, IOSYMBOL("source"),
        wrapObject<Ptr<IActor> >(armament->getSourceActor(), IOSTATE));
    IoObject_setSlot_to_(self, IOSYMBOL("target"),
        wrapObject<Ptr<IActor> >(targeter->getCurrentTarget(), IOSTATE));
    IoObject_setSlot_to_(self, IOSYMBOL("missile"),
        wrapObject<Ptr<IActor> >(projectile, IOSTATE));

    Ptr<IActor> target = targeter->getCurrentTarget();
    if (target) target->message("missileShot", self);

    IoState_popRetainPool(IOSTATE);
    
    return projectile;
}
