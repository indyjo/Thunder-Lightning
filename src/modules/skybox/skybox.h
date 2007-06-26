#include <tnl.h>
#include <modules/environment/environment.h>
#include <modules/texman/TextureManager.h>
#include <interfaces/IGame.h>
#include <interfaces/ISkyBox.h>

class SkyBox: public ISkyBox
{
public:
    SkyBox(IGame *thegame);

    virtual void draw();

private:
    IGame *thegame;
    Ptr<IConfig> config;
    JRenderer *renderer;
    TexPtr textures[6];
};
