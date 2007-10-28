#ifndef SPECIALEFFECTS_H
#define SPECIALEFFECTS_H

#include <modules/math/Vector.h>
#include <object.h>
struct IGame;
struct IActor;

void aircraftFirstExplosion(Ptr<IGame> game, Ptr<IActor> aircraft);
void aircraftFinalExplosion(Ptr<IGame> game, Ptr<IActor> aircraft);
void tankFirstExplosion(Ptr<IGame> game, Ptr<IActor> tank);
void tankFinalExplosion(Ptr<IGame> game, Ptr<IActor> tank);
void tankCannonFire(Ptr<IGame> game, Vector pos, Vector dir);
void tankMachineGunFire(Ptr<IGame> game, Vector pos, Vector dir);
void carrierMachineGunFire(Ptr<IGame> game, Vector pos, Vector dir);

#endif
