#ifndef TANK_H
#define TANK_H

#include <landscape.h>
#include <modules/math/Vector.h>
#include <modules/math/Matrix.h>
#include <modules/model/Skeleton.h>
#include <modules/actors/simpleactor.h>
#include <modules/engines/tankengine.h>
#include <modules/engines/controls.h>
#include <modules/weaponsys/Armament.h>

class TankBrain;
class SoundSource;
class EventSheet;
class Targeter;

class Tank : virtual public SimpleActor, virtual public SigObject {
public:
    Tank(Ptr<IGame> thegame);
    ~Tank();

    virtual void action();

    virtual void draw();

    virtual void applyDamage(float damage, int domain, Ptr<IProjectile>);
    
    virtual void setLocation(const Vector & p);

    virtual int getNumViews();
    virtual Ptr<IView> getView(int n);

    void explode();

    void shoot();

    virtual bool hasControlMode(ControlMode);
    virtual void setControlMode(ControlMode);
    
private:
    JRenderer * renderer;
    Ptr<ITerrain> terrain;

    Ptr<Skeleton> skeleton;

    // sound sources
    Ptr<SoundSource> sound_low, sound_high;

    // Tank state
    Armament armament;
    ControlMode control_mode;
    Ptr<EventSheet> event_sheet;
    Ptr<TankControls> tank_controls;
    Ptr<TankEngine> tank_engine;
    Ptr<TankBrain> brain;
    Ptr<Targeter> targeter;
    float damage;
    double age;
};




#endif
