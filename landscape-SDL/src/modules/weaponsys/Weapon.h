#ifndef WEAPON_H
#define WEAPON_H

#include <string>
#include <vector>
#include <landscape.h>
#include <interfaces/IWeapon.h>
#include <modules/model/Skeleton.h>

struct IPositionProvider;
struct Armament;

class Weapon : public IWeapon {
protected:
    struct Barrel;

    std::string name;
    int rounds, maxrounds;
    float loadtime;
    bool singleshot, triggered;
    
    std::vector<Barrel> barrels;
    int next_barrel;
public:
    Weapon(const std::string & name, int rounds, float loadtime, bool singleshot=false);
    
    void addBarrel(Ptr<IPositionProvider>);
    
    virtual const char * getName();
    
    virtual int getMaxRounds();
    virtual int getRoundsLeft();
    
    /// Child classes MAY override this to implement special locking behavior
    /// The default implementation just returns whether there is ammo left and reload has finished
    virtual bool canFire();
    
    virtual void trigger(Armament &);
    virtual void release();
    
    /// Child classes MUST override this to fire the projectile or missile
    virtual void onFire(Armament &)=0;
    
    virtual void draw(JRenderer*);
    
    virtual void action(float delta_t, Armament &);
};

struct Weapon::Barrel {
    Ptr<IPositionProvider> position;
    float secs_since_fire;
};
  

#endif
