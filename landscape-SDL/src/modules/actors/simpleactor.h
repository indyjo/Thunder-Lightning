#ifndef SIMPLEACTOR_H
#define SIMPLEACTOR_H

#include <string>
#include <vector>
#include <landscape.h>
#include <TargetInfo.h>
#include <Faction.h>
#include <modules/engines/engine.h>
#include <interfaces/IActor.h>
#include <interfaces/IGame.h>
#include <interfaces/IView.h>

class SimpleActor : virtual public IActor
{
protected:
    class RelativeView;

    Ptr<IGame> thegame;
    Ptr<IEngine> engine;
    State state;
    std::vector<Ptr<IView> > views;
    Ptr<TargetInfo> target_info;
    Ptr<Faction> faction;
public:
    SimpleActor( Ptr<IGame> game);

    inline void setEngine(Ptr<IEngine> e) { engine = e; }
    inline Ptr<IEngine> getEngine() { return engine; }
    inline void setTargetInfo(Ptr<TargetInfo> ti) { target_info = ti; }

    // IPositionProvider
    virtual Vector getLocation();
    virtual Vector getFrontVector();
    virtual Vector getRightVector();
    virtual Vector getUpVector();
    virtual void getOrientation(Vector * up, Vector * right, Vector * front);

    // IMovementProvider
    virtual Vector getMovementVector();

    // IPositionReceiver
    virtual void setLocation(const Vector &);
    virtual void setOrientation(const Vector & up,
                                const Vector & right,
                                const Vector & front);

    // IMovementReceiver
    virtual void setMovementVector(const Vector &);

    // IActor
    virtual Ptr<TargetInfo> getTargetInfo();
    virtual Ptr<Faction> getFaction();
    virtual void setFaction(Ptr<Faction>);
    virtual void action();
    virtual State getState();
    virtual float getRelativeDamage();
    virtual void applyDamage(float damage, int domain);
    virtual int getNumViews();
    virtual Ptr<IView> getView(int n);
    virtual bool hasControlMode(ControlMode);
    virtual void setControlMode(ControlMode);

    // IDrawable
    virtual void draw();
};

class SimpleActor::RelativeView : virtual public IView {
    IActor &subject;
    Vector p,up,right,front;
    Ptr<IDrawable> gunsight;
public:
    inline RelativeView(IActor & subject,
        const Vector & p,
        const Vector & right,
        const Vector & up,
        const Vector & front,
        Ptr<IDrawable> gunsight=0)
    :   subject(subject),
        p(p),
        up(up), right(right), front(front),
        gunsight(gunsight)
    { }

    // IPositionProvider
    virtual Vector getLocation();
    virtual Vector getFrontVector();
    virtual Vector getRightVector();
    virtual Vector getUpVector();
    virtual void getOrientation(Vector * up, Vector * right, Vector * front);
    
    // IMovementProvider
    virtual Vector getMovementVector();
    
    // IView
    virtual Ptr<IActor> getViewSubject();
    virtual Ptr<IDrawable> getGunsight();
};

#endif
