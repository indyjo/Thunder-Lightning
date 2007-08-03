#ifndef GUNSIGHT_H
#define GUNSIGHT_H

#include <interfaces/IGame.h>
#include <modules/jogi/JRenderer.h>
#include <modules/ui/Panel.h>

struct IGame;
struct ICamera;
struct IMovementProvider;
class FlightInfo;
class Targeter;
class Armament;
class SimpleActor;
class DataNode;

class FlexibleGunsight : public UI::Panel {
    Ptr<IGame> thegame;
public:
    FlexibleGunsight(Ptr<IGame>);
    virtual ~FlexibleGunsight();

    virtual void draw(const UI::Surface & surface);
    
    // Gunsight configuration
    void addDebugInfo(Ptr<IGame>, Ptr<IActor>);
    void addBasics(Ptr<IGame>, Ptr<IActor>);
    void addBasicCrosshairs(const char * parent="screen");
    void addFlightModules(Ptr<IGame>, FlightInfo &, Ptr<DataNode>);
    void addTargeting(Ptr<IMovementProvider>, Ptr<Targeter>, Ptr<Armament>);
    void addArmamentToScreen(Ptr<IGame>, Ptr<Armament>, size_t);
    void addArmamentToScreenRight(Ptr<IGame>, Ptr<Armament>, size_t);
    void addDirectionOfFlight(Ptr<IActor>);
    void addInterception(Ptr<IActor> src, Ptr<IActor> target);
    void addMissileWarning(Ptr<IGame> game, Ptr<SimpleActor> actor);
    void addInfoMessage(Ptr<IGame> game);
};

#endif
