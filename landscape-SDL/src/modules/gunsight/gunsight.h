#include <math.h>
#include <stdio.h>
#include <landscape.h>
#include <modules/environment/environment.h>
#include <modules/flight/flightinfo.h>

class Gunsight: public IGunsight, virtual public SigObject
{
public:
    Gunsight(IGame *thegame);

    virtual void draw();

    virtual void enable();
    virtual void disable();
    
    virtual Ptr<IActor> getCurrentTarget();

protected:
    void drawGunsight();
    void drawThrustBar();
    void drawFlightInfo();
    void drawTargetInfo();
    void drawTargets();
    bool targetSelectable(Ptr<IActor>);
    void selectNextTarget();
    void selectTargetInGunsight();
    void toggleInfo();
private:
    IGame * thegame;
    JRenderer *renderer;
    TexPtr tex;
    bool enabled;
    bool display_info;
    Ptr<IPlayer> player;
    FlightInfo fi;
    Ptr<ITerrain> terrain;
    Ptr<IActor> current_target;
};
