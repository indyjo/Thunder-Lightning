#include <stdio.h>
#include <math.h>
#include <fstream.h>
#include <landscape.h>
#include <modules/environment/environment.h>

class SkyBox: public ISkyBox
{
public:
    SkyBox(IGame *thegame);

    virtual void draw();

private:
    IGame *thegame;
    Ptr<IConfig> config;
    Ptr<ICamera> cam;
    JRenderer *renderer;
    TexPtr textures[6];
};
