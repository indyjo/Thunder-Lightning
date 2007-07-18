#ifndef WEAPON_H
#define WEAPON_H

#include <string>
#include <vector>
#include <tnl.h>
#include <interfaces/IConfig.h>
#include <interfaces/IWeapon.h>
#include <modules/model/Skeleton.h>

struct IPositionProvider;
class Armament;

class Weapon : public IWeapon {
protected:
    struct Barrel;

    Armament * armament;
    
    std::string name;
    int rounds, maxrounds;
    float loadtime;
    bool singleshot, triggered;
    bool is_guided;
    float max_range;
    float reference_speed;
    float time_of_accel;
    
    
    std::vector<Barrel> barrels;
    int next_barrel;
    
    SigC::Signal1<void, Ptr<IWeapon> > fire_signal;
    WeakPtr<IActor> last_fired_round;
public:
    Weapon(Ptr<IConfig>, const std::string & name, int rounds);
    
    /// Set the weapon's associated armament.
    /// This member function is only called by Armament::addWeapon
    inline void attachToArmament(Armament & arms) {
        armament = &arms;
    }
    
    void addBarrel(Ptr<IPositionProvider>);
    
    virtual const char * getName();
    
    virtual int getMaxRounds();
    virtual int getRoundsLeft();
    
    virtual void setMaxRounds(int);
    virtual void setRoundsLeft(int);
    
    /// Child classes MAY override this to implement special locking behavior
    /// The default implementation just returns whether there is ammo left and reload has finished
    virtual bool canFire();
    
    virtual void trigger();
    virtual void release();
    virtual bool isTriggered();
    
    /// Child classes MUST override this to fire the projectile or missile
    virtual WeakPtr<IActor> onFire()=0;
    
    virtual void draw(JRenderer*);
    virtual void action(float delta_t);

    virtual bool  isGuided();
    virtual float maxRange();
    virtual float referenceSpeed();
    virtual float timeOfAcceleration();
    
    virtual WeakPtr<IActor> lastFiredRound();
    virtual SigC::Signal1<void, Ptr<IWeapon> > onFireSig();
};

struct Weapon::Barrel {
    Ptr<IPositionProvider> position;
    float secs_since_fire;
};
  

#endif
