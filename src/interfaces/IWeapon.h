#ifndef IWEAPON_H
#define IWEAPON_H

#include <interfaces/IActor.h>
#include <modules/jogi/JRenderer.h>
#include <Weak.h>

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
    
    // Information necessary for HUD and AI
    virtual bool  isGuided()=0;
    virtual float maxRange()=0;
    
    /// Speed that the round would attain if fired from a still-standing point
    virtual float referenceSpeed()=0;
    /// Duration of the round's acceleration phase until reference speed is reached
    virtual float timeOfAcceleration()=0;
    
    virtual WeakPtr<IActor> lastFiredRound()=0;
    virtual SigC::Signal1<void, Ptr<IWeapon> > onFireSig()=0;
};

#endif
