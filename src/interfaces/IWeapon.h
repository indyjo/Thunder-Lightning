#ifndef IWEAPON_H
#define IWEAPON_H

#include <modules/jogi/JRenderer.h>
#include <object.h>

class Armament;

struct IWeapon : public Object {
    virtual const char * getName()=0;
    
    virtual int getMaxRounds()=0;
    virtual int getRoundsLeft()=0;
    
    virtual void setMaxRounds(int)=0;
    virtual void setRoundsLeft(int)=0;
    
    virtual bool canFire()=0;
    virtual void trigger()=0;
    virtual void release()=0;
    
    virtual void action(float delta_t)=0;
    virtual void draw(JRenderer*)=0;
};

#endif
