#include <cstdio>
#include <landscape.h>

class Horizon: public IHorizon
{
public:
    Horizon(IGame *thegame);

    virtual void draw();

private:
    IGame *thegame;
    JRenderer *renderer;
    Ptr<IPlayer> player;
    Ptr<ISky> sky;
};
