#ifndef WEAPON_H
#define WEAPON_H

#include <string>
#include <vector>
#include <landscape.h>
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
    
    std::vector<Barrel> barrels;
    int next_barrel;
public:
    Weapon(const std::string & name, int rounds, float loadtime, bool singleshot=false);
    
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
    virtual void onFire()=0;
    
    virtual void draw(JRenderer*);
    virtual void action(float delta_t);
};

struct Weapon::Barrel {
    Ptr<IPositionProvider> position;
    float secs_since_fire;
};
  

#endif
