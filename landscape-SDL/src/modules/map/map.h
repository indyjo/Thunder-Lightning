#include <landscape.h>
#include <modules/environment/environment.h>

class Map: public IMap, public SigObject
{
public:
    Map(IGame *thegame);

    virtual void draw();

    virtual void enable();
    virtual void disable();

    virtual void magnify();
    virtual void demagnify();

private:
    void drawPlayerSymbol(float x, float y, float z);
    Ptr<IGame> thegame;
    JRenderer *renderer;
    Ptr<Environment> env;
    jrtxtid_t txtid;
    int enabled;
    int magnified;
    float t;
};
