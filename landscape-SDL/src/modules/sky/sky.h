#include <math.h>
#include <fstream.h>
#include <landscape.h>

class Sky: public ISky
{
public:
    Sky(IGame *thegame);

    virtual void draw();

    virtual void getSkyColor(jcolor3_t *col);

private:
    void getCurrentUV(int *u, int *v);
    void draw(int txt);

private:
    IGame *thegame;
    Ptr<IPlayer> player;
    JRenderer *renderer;
    TexPtr txtid1,txtid2;
};
