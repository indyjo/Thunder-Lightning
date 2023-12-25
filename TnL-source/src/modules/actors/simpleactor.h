#ifndef SIMPLEACTOR_H
#define SIMPLEACTOR_H

#include <string>
#include <vector>
#include <tnl.h>
#include <TargetInfo.h>
#include <Faction.h>
#include <modules/engines/engine.h>
#include <interfaces/IActor.h>
#include <interfaces/IGame.h>
#include <interfaces/IView.h>
#include <modules/math/Transform.h>
#include <modules/model/model.h>
#include <modules/model/Skeleton.h>

class Armament;
class Targeter;
class EventSheet;
class SimpleView;

class SimpleActor : virtual public IActor, virtual public SigObject
{
protected:
    Ptr<IGame> thegame;
#ifdef HAVE_IO
    IoObject *self;
#endif
    Ptr<IEngine> engine;
    State state;
    Ptr<TargetInfo> target_info;
    Ptr<Faction> faction;
    Ptr<Model> model;
    Ptr<Skeleton> skeleton;
    Ptr<DataNode> controls;
    Ptr<Armament> armament;
    Ptr<Targeter> targeter;
    ControlMode control_mode;
    bool is_linked;
private:
    Ptr<EventSheet> event_sheet;
public:
    SimpleActor( Ptr<IGame> game);
    virtual ~SimpleActor();

    void createIoObject();
#ifdef HAVE_IO
    void setIoObject(IoObject *self);
    /// Called by Io object's free function
    inline void rawResetIoObject() { self=0; }
#endif

    void onLinked();
    void onUnlinked();
    inline bool isLinked() { return is_linked; }

    inline Ptr<DataNode> getControls() { return controls; }
    
    inline void setEngine(Ptr<IEngine> e) { engine = e; engine->setControls(controls); }
    inline Ptr<IEngine> getEngine() { return engine; }
    inline void setTargetInfo(Ptr<TargetInfo> ti) { target_info = ti; }
    
    inline void setModel(Ptr<Model> m) {model = m;}
    inline Ptr<Model> getModel() {return model;}

    inline void setSkeleton(Ptr<Skeleton> s) {skeleton = s;}
    inline Ptr<Skeleton> getSkeleton() {return skeleton;}
    
    void setArmament(Ptr<Armament> armament);
    Ptr<Armament> getArmament();
    
    void setTargeter(Ptr<Targeter> targeter);
    Ptr<Targeter> getTargeter();
    
    // Gets the current event sheet. Creates one if not called before.
    Ptr<EventSheet> getEventSheet();
    void mapArmamentEvents();
    void mapTargeterEvents();
    
    // Prepares a view object to handle view-specific events like "gunsight-target"
    void mapViewEvents(Ptr<SimpleView> view);
    // Called when the user has requested to select target in gunsight
    // with 'G'. The Actor must decide whether to proceed or to ignore.
    virtual void onSelectTargetInView(IView * view);

#ifdef HAVE_IO
    typedef SigC::Signal2<void, std::string, IoObject *> MessageSignal;
    /// Emitted when a message was received
    MessageSignal message_signal;
#endif

    // IPositionProvider
    virtual Vector getLocation();
    virtual Vector getFrontVector();
    virtual Vector getRightVector();
    virtual Vector getUpVector();
    virtual void getOrientation(Vector * up, Vector * right, Vector * front);
    // utility functions
    Matrix3 getOrientationAsMatrix();
    Quaternion getOrientationAsQuaternion();
    Transform getTransform();

    // IMovementProvider
    virtual Vector getMovementVector();

    // IPositionReceiver
    virtual void setLocation(const Vector &);
    virtual void setOrientation(const Vector & up,
                                const Vector & right,
                                const Vector & front);
    // utility functions
    void setOrientation(const Matrix3 &);
    void setOrientation(const Quaternion &);
    void setTransform(const Transform &);

    // IMovementReceiver
    virtual void setMovementVector(const Vector &);

    // IActor
    virtual Ptr<TargetInfo> getTargetInfo();
    virtual Ptr<Faction> getFaction();
    virtual void setFaction(Ptr<Faction>);
    virtual void action();
    virtual State getState();
    virtual void kill();
    virtual float getRelativeDamage();
    virtual void applyDamage(float damage, int domain, Ptr<IProjectile>);
    virtual int getNumViews();
    virtual Ptr<IView> getView(int n);
    virtual bool hasControlMode(ControlMode);
    virtual void setControlMode(ControlMode);
    virtual ControlMode getControlMode();
#ifdef HAVE_IO
    virtual IoObject* message(std::string name, IoObject* args);
    virtual IoObject* getIoObject();
#endif
    // IDrawable
    virtual void draw();
};

#endif
