#ifndef IACTOR_H
#define IACTOR_H

#include <interfaces/IDrawable.h>
#include <interfaces/IMovementProvider.h>
#include <interfaces/IMovementReceiver.h>

class Faction;
class TargetInfo;

class IActor:   virtual public IDrawable,
              	virtual public IMovementProvider,
              	virtual public IMovementReceiver
{
public:
    typedef enum {ALIVE, DEAD} State;

    virtual Ptr<TargetInfo> getTargetInfo()=0;

    virtual Ptr<Faction> getFaction()=0;
    virtual void setFaction(Ptr<Faction>)=0;

    virtual void action()=0;
    virtual State getState()=0;

    virtual float getRelativeDamage()=0;
    virtual void applyDamage(float damage, int domain=0)=0;

    virtual int getNumViews()=0;
    virtual Ptr<IPositionProvider> getView(int n)=0;
};

#endif
