#ifndef IACTOR_H
#define IACTOR_H

#include <interfaces/IDrawable.h>
#include <interfaces/IMovementProvider.h>
#include <interfaces/IMovementReceiver.h>

class Faction;
class TargetInfo;
class IView;
class IProjectile;

class IActor:   virtual public IDrawable,
              	virtual public IMovementProvider,
              	virtual public IMovementReceiver
{
public:
    typedef enum {ALIVE, DEAD} State;
    typedef enum {UNCONTROLLED, MANUAL, AUTOMATIC} ControlMode;

    virtual Ptr<TargetInfo> getTargetInfo()=0;

    virtual Ptr<Faction> getFaction()=0;
    virtual void setFaction(Ptr<Faction>)=0;

    virtual void action()=0;
    
    virtual void kill()=0;
    virtual State getState()=0;
    inline bool isAlive() { return getState()==ALIVE; }

    virtual float getRelativeDamage()=0;
    virtual void applyDamage(float damage,
    	int domain=0,
    	Ptr<IProjectile> projectile=0)=0;

    virtual int getNumViews()=0;
    virtual Ptr<IView> getView(int n)=0;
    
    virtual bool hasControlMode(ControlMode)=0;
    virtual void setControlMode(ControlMode)=0;
};

#endif
