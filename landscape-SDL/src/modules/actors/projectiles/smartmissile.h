#include <landscape.h>
#include <modules/math/Vector.h>
#include <modules/actors/fx/spark.h>
#include <modules/actors/fx/smoketrail.h>
#include <modules/actors/fx/smokecolumn.h>
#include <modules/math/Rendezvous.h>
#include <modules/actors/simpleactor.h>

class SmartMissileEngine;
class SoundSource;


class TargetMarker : public SimpleActor
{
public:
    TargetMarker(Ptr<IGame> game)
    : SimpleActor(game)
    {
        setTargetInfo(new TargetInfo(
            "Target marker", 1.0f, TargetInfo::DEBUG));
    }
    
    virtual void action() { }
    virtual void draw() { }
    virtual void kill() { state = DEAD; }
    virtual void setPos(const Vector & pos) { setLocation(pos); }
};

class SmartMissile: public SimpleActor
{
public:
    SmartMissile(Ptr<IGame> thegame, Ptr<IActor> target);

    virtual void action();

    virtual void draw();
    
    virtual void shoot(const Vector &pos, const Vector &vec, const Vector &dir);
    
    virtual void hitTarget(float damage);

private:
    void explode();

private:
    double age;
    JRenderer *renderer;
    Ptr<IActor> target;
    Ptr<ITerrain> terrain;
    Ptr<TargetMarker> marker;
    Rendezvous rendezvous;
    float damage;
    Ptr<SmartMissileEngine> engine;
    Ptr<SoundSource> engine_sound_src;
};
