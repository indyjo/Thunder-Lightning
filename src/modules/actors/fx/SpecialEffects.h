#ifndef SPECIALEFFECTS_H
#define SPECIALEFFECTS_H

#include <object.h>
struct IGame;
struct IActor;

void aircraftFirstExplosion(Ptr<IGame> game, Ptr<IActor> aircraft);
void aircraftFinalExplosion(Ptr<IGame> game, Ptr<IActor> aircraft);
void tankFirstExplosion(Ptr<IGame> game, Ptr<IActor> tank);
void tankFinalExplosion(Ptr<IGame> game, Ptr<IActor> tank);

#endif
