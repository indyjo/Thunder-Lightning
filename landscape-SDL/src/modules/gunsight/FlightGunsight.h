#ifndef FLIGHT_GUNSIGHT_H
#define FLIGHT_GUNSIGHT_H

#include "gunsight.h"

struct IFontMan;
struct IGame;
struct FlightInfo;

struct FlightGunsight : public FlexibleGunsight {
	FlightGunsight(Ptr<IGame>, FlightInfo &);
};

struct CrosshairModule : public GunsightModule {
	CrosshairModule();
    void draw(FlexibleGunsight &);
};

struct HUDFrameModule : public GunsightModule {
	HUDFrameModule(float screen_x, float screen_y);
    void draw(FlexibleGunsight &);
};

class SpeedModule : public GunsightModule {
	FlightInfo& flight_info;
	Ptr<IFontMan> fontman;
public:
	SpeedModule(Ptr<IGame>, FlightInfo&);
    void draw(FlexibleGunsight &);
};

class HeightModule : public GunsightModule {
	FlightInfo& flight_info;
	Ptr<IFontMan> fontman;
public:
	HeightModule(Ptr<IGame>, FlightInfo&);
    void draw(FlexibleGunsight &);
};

#endif
