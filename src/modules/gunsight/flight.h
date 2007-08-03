#ifndef FLIGHT_GUNSIGHT_H
#define FLIGHT_GUNSIGHT_H

#include "gunsight.h"

struct IFontMan;
struct IGame;
class FlightInfo;
class JRenderer;

struct CrosshairModule : public UI::Component {
	CrosshairModule();
    void draw(UI::Panel &);
};

struct HUDFrameModule : public UI::Component {
	HUDFrameModule();
    virtual void draw(UI::Panel &);
    virtual void onLayout(UI::Panel &);
};

class SpeedModule : public UI::Component {
	FlightInfo& flight_info;
	Ptr<IFontMan> fontman;
public:
	SpeedModule(Ptr<IGame>, FlightInfo&);
    void draw(UI::Panel &);
};

class HeightModule : public UI::Component {
	FlightInfo& flight_info;
	Ptr<IFontMan> fontman;
public:
	HeightModule(Ptr<IGame>, FlightInfo&);
    void draw(UI::Panel &);
};

class HeightGraphModule : public UI::Component {
    FlightInfo& flight_info;
    Ptr<IFontMan> fontman;
public:
    HeightGraphModule(Ptr<IGame>, FlightInfo&);
    void draw(UI::Panel &);
    virtual void onLayout(UI::Panel &);
};

class SpeedGraphModule : public UI::Component {
    FlightInfo& flight_info;
    Ptr<IFontMan> fontman;
public:
    SpeedGraphModule(Ptr<IGame>, FlightInfo&);
    void draw(UI::Panel &);
    virtual void onLayout(UI::Panel &);
};

class HorizonIndicator : public UI::Component {
	FlightInfo& flight_info;
	Ptr<IFontMan> fontman;
public:
	HorizonIndicator(Ptr<IGame>, FlightInfo &, float w, float h);
	void draw(UI::Panel &);
	void drawIndicator(JRenderer *, int degrees, Vector center, Vector right);
};

class GearHookIndicator : public UI::Component {
	Ptr<DataNode> controls;
	Ptr<IFontMan> fontman;
public:
	GearHookIndicator(Ptr<IGame>, Ptr<DataNode> controls);
	void draw(UI::Panel &);
};


#endif
