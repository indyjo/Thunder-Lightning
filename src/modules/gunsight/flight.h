#ifndef FLIGHT_GUNSIGHT_H
#define FLIGHT_GUNSIGHT_H

#include "gunsight.h"

struct IFontMan;
struct IGame;
class FlightInfo;
class JRenderer;

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

class HeightGraphModule : public GunsightModule {
    FlightInfo& flight_info;
    Ptr<IFontMan> fontman;
public:
    HeightGraphModule(float screen_h, Ptr<IGame>, FlightInfo&);
    void draw(FlexibleGunsight &);
};

class SpeedGraphModule : public GunsightModule {
    FlightInfo& flight_info;
    Ptr<IFontMan> fontman;
public:
    SpeedGraphModule(float screen_h, Ptr<IGame>, FlightInfo&);
    void draw(FlexibleGunsight &);
};

class HorizonIndicator : public GunsightModule {
	FlightInfo& flight_info;
	Ptr<IFontMan> fontman;
public:
	HorizonIndicator(Ptr<IGame>, FlightInfo &, float w, float h);
	void draw(FlexibleGunsight &);
	void drawIndicator(JRenderer *, int degrees, Vector center, Vector right);
};

class GearHookIndicator : public GunsightModule {
	Ptr<DataNode> controls;
	Ptr<IFontMan> fontman;
public:
	GearHookIndicator(Ptr<IGame>, Ptr<DataNode> controls);
	void draw(FlexibleGunsight &);
};


#endif
