#ifndef GUNSIGHT_DEBUG_H
#define GUNSIGHT_DEBUG_H

#include "gunsight.h"

struct IFontMan;
struct IGame;

class FPSModule : public GunsightModule {
	Ptr<IGame> game;
	float fps;
public:
	FPSModule(Ptr<IGame> game);
    void draw(FlexibleGunsight &);
};

class TargetInfoModule : public GunsightModule {
	Ptr<IGame> game;
	Ptr<IActor> actor;
public:
	TargetInfoModule(Ptr<IGame>, Ptr<IActor>);
	void draw(FlexibleGunsight &);
};


#endif

