#include <tnl.h>
#include <interfaces/IGame.h>
#include <interfaces/ICamera.h>
#include <modules/jogi/JRenderer.h>

#include "gunsight.h"

FlexibleGunsight::FlexibleGunsight(Ptr<IGame> game)
:   UI::Panel(game->getRenderer()), thegame(game)
{
}

FlexibleGunsight::~FlexibleGunsight()
{
}

void FlexibleGunsight::draw(const UI::Surface & surface) {
    UI::Panel::draw(surface);
}

