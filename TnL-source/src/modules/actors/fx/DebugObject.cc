#include "DebugObject.h"
#include <TargetInfo.h>
#include <modules/clock/clock.h>
#include <game.h>

DebugActor::DebugActor(Ptr<IGame> game, const Vector & p, const char * text, float ttl)
:   SimpleActor(game), ttl(ttl)
{
    setTargetInfo(new TargetInfo(
        "DebugObject", 0.1f, TargetInfo::DEBUG));
    getTargetInfo()->setTargetInfo(text);
    game->addActor(this);
    setLocation(p);
}

DebugActor::DebugActor(const Vector & p, const char * text, float ttl)
:   SimpleActor(Game::the_game), ttl(ttl)
{
    setTargetInfo(new TargetInfo(
        "DebugObject", 0.1f, TargetInfo::DEBUG));
    getTargetInfo()->setTargetInfo(text);
    Game::the_game->addActor(this);
    setLocation(p);
}

void DebugActor::action() {
    float delta_t = thegame->getClock()->getStepDelta();
    ttl -= delta_t;
    if(ttl < 0) state=DEAD;
    //SimpleActor::action();
}
