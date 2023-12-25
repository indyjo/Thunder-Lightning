#ifndef IACTOR_H
#define IACTOR_H

#ifdef HAVE_IO
#include <IoObject.h>
#endif
#include <string>
#include <interfaces/IDrawable.h>
#include <interfaces/IMovementProvider.h>
#include <interfaces/IMovementReceiver.h>
#include <Weak.h>

class Faction;
class TargetInfo;
struct IView;
struct IProjectile;

struct IActor:   virtual public IDrawable,
              	 virtual public IMovementProvider,
              	 virtual public IMovementReceiver,
                 virtual public Weak
{
public:
    typedef enum {ALIVE, DEAD} State;
    typedef enum {UNCONTROLLED, MANUAL, AUTOMATIC} ControlMode;

    /// Called when added to an ActorStage
    virtual void onLinked() =0;
    /// Called when removed from an ActorStage
    virtual void onUnlinked() =0;
    
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
    virtual ControlMode getControlMode()=0;

#ifdef HAVE_IO
    /// Generic interface for sending specific messages to actors which can be handled from Io
    virtual IoObject* message(std::string name, IoObject* args)=0;
    /// Returns the Io Object that represents this actor
    /// Actors may cache an Io object or return a newly created one on each call.
    virtual IoObject* getIoObject()=0;
#endif
};

#endif
