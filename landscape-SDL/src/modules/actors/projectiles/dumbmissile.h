#include <landscape.h>
#include <modules/math/Vector.h>
#include <modules/actors/fx/spark.h>
#include <modules/actors/fx/smoketrail.h>
#include <modules/actors/fx/smokecolumn.h>
#include <modules/actors/simpleactor.h>
#include <modules/engines/missileengine.h>


class DumbMissile: public SimpleActor, public IProjectile
{
public:
    DumbMissile(Ptr<IGame> thegame);

    virtual void action();

    virtual void draw();
    
    virtual void shoot(const Vector &pos, const Vector &vec, const Vector &dir);

private:
    void shootSparks();

private:
    Ptr<MissileEngine> engine;
    double age;
    JRenderer *renderer;
    Ptr<ITerrain> terrain;
    Ptr<ICamera> camera;
};
