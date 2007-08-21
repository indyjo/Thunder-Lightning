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
    
    // Gunsight configuration.
    // These functions will configure components specific to certain actors
    // or certain aspects and can be used to assemble custom HUDs.
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

    // These are meant for the static overlay that will always be rendered.
    // They will not be required for individual HUD configurations but will
    // be enabled by default inside class Game.
    void addStaticDebugInfo(Ptr<IGame> game);
    void addStaticInfoMessage(Ptr<IGame> game);
};

#endif
