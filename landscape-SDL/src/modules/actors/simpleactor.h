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

class SimpleActor : virtual public IActor
{
protected:
    class RelativeView;

    Ptr<IGame> thegame;
    Ptr<IEngine> engine;
    State state;
    std::vector<Ptr<IPositionProvider> > views;
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
    virtual Ptr<IPositionProvider> getView(int n);

    // IDrawable
    virtual void draw();
};

class SimpleActor::RelativeView : virtual public IPositionProvider {
    IPositionProvider &base;
    Vector p,up,right,front;
public:
    inline RelativeView(IPositionProvider &base,
        const Vector & p,
        const Vector & right,
        const Vector & up,
        const Vector & front)
    :   base(base),
        p(p),
        up(up), right(right), front(front)
    { }

    virtual Vector getLocation();
    virtual Vector getFrontVector();
    virtual Vector getRightVector();
    virtual Vector getUpVector();
    virtual void getOrientation(Vector * up, Vector * right, Vector * front);
};

#endif
