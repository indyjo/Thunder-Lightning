#ifndef IWEAPON_H
#define IWEAPON_H

#include <modules/jogi/JRenderer.h>
#include <object.h>

struct Armament;

struct IWeapon : public Object {
    virtual const char * getName()=0;
    
    virtual int getMaxRounds()=0;
    virtual int getRoundsLeft()=0;
    
    virtual bool canFire()=0;
    virtual void trigger(Armament &)=0;
    virtual void release()=0;
    
    virtual void action(float delta_t, Armament &)=0;
    virtual void draw(JRenderer*)=0;
};

#endif
