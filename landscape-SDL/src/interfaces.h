#ifndef _INTERFACES_H
#define _INTERFACES_H

#include <list>

class IGame;
class IActorStage;
class IMap;
class ITerrain;
class ISky;
class IHorizon;
class ISkyBox;
class IGunsight;
class ILoDQuadManager;
class IConfig;
class IPositionProvider;
class IMovementProvider;
class IPositionReceiver;
class IMovementReceiver;
class ICamera;
class IActor;
class IPlayer;
class IProjectile;
class IFollower;
class IFontMan;
class IModelMan;

class Clock;
class Environment;

namespace Collide { class CollisionManager; }

class IActorStage : virtual public Object
{
public:
    typedef std::list< Ptr<IActor> > ActorList;

    virtual void addActor(Ptr<IActor>)=0;
    virtual const ActorList & getActorList()=0;
    virtual void queryActorsInSphere(ActorList &, const Vector &, float)=0;
    virtual void queryActorsInCylinder(ActorList &, const Vector &, float)=0;
    virtual void queryActorsInBox(ActorList &, const Vector &, const Vector &)=0;
};


class IGame : virtual public Object, virtual public IActorStage
{
public:
    virtual Ptr<TextureManager> getTexMan()=0;
    virtual JRenderer *getRenderer()=0;
    virtual EventRemapper *getEventRemapper()=0;
    virtual Ptr<IModelMan> getModelMan()=0;
    virtual Ptr<IConfig> getConfig()=0;
    virtual Ptr<IActor> getCamPos()=0;
    virtual Ptr<ICamera> getCamera()=0;
    virtual Ptr<Clock> getClock()=0;
    virtual Ptr<ITerrain> getTerrain()=0;
    virtual Ptr<IPlayer> getPlayer()=0;
    virtual Ptr<IGunsight> getGunsight()=0;
    virtual Ptr<Environment> getEnvironment()=0;
#if ENABLE_SKY
    virtual Ptr<ISky> getSky()=0;
#endif
    virtual Ptr<IFontMan> getFontMan()=0;
    virtual Ptr<SoundMan> getSoundMan()=0;
    virtual Ptr<Collide::CollisionManager> getCollisionMan()=0;
    virtual void getMouseState(float *mx, float *my, int *buttons)=0;
    virtual double  getTimeDelta()=0;
    virtual double  getTime()=0;
    virtual void drawDebugTriangleAt(const Vector & p) = 0;

    virtual void clearScreen()=0;
};


class IDrawable : virtual public Object
{
public:
    virtual void draw()=0;
};

class IMap: public IDrawable
{
public:
    virtual void enable()=0;
    virtual void disable()=0;

    virtual void magnify()=0;
    virtual void demagnify()=0;
};

class ITerrain: public IDrawable
{
public:
    virtual float getHeightAt(float x, float z)=0;
    // lineCollides: test if line between a and b intersects terrain
    // Stores intersection point in x
    virtual bool lineCollides(Vector a, Vector b, Vector * x)=0;
};

class ISky: public IDrawable
{
public:
    virtual void getSkyColor(jcolor3_t *col)=0;
};

class IHorizon: public IDrawable
{
};


class ISkyBox: public IDrawable
{
};


class IGunsight: public IDrawable
{
public:
    virtual void enable()=0;
    virtual void disable()=0;
    virtual Ptr<IActor> getCurrentTarget()=0;
};

class ILoDQuadManager: public ITerrain
{
};

class IConfig : virtual public Object
{
public:
    virtual void feedArguments(int argc, const char **const argv)=0;
    
    virtual const char *query(const char *key, const char *def =0)=0;
};

class IPositionProvider : virtual public Object
{
public:
    virtual Vector getLocation() = 0;
    virtual Vector getFrontVector() = 0;
    virtual Vector getRightVector() = 0;
    virtual Vector getUpVector() = 0;
    virtual void getOrientation(  Vector * up,
                                    Vector * right,
                                    Vector * front) = 0;
};

class IMovementProvider: virtual public IPositionProvider
{
public:
    virtual Vector getMovementVector()=0;
};

class IPositionReceiver : virtual public Object
{
public:
    virtual void setLocation(const Vector &) = 0;
    virtual void setOrientation(const Vector & up,
                                const Vector & right,
                                const Vector & front) = 0;
};

class IMovementReceiver: virtual public IPositionReceiver
{
public:
    virtual void setMovementVector(const Vector &)=0;
};


class ICamera: virtual public IPositionProvider
{
public:
    virtual void alignWith(IPositionProvider *pos_provider)=0;

    virtual void getCamera(JCamera *camera)=0;

    virtual void getFrontBackPlane(float plane[4])=0;
    virtual void getFrustumPlanes(float planes[6][4])=0;
    virtual float getFocus()=0;
    virtual float getAspect()=0;
};

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

class IPlayer: virtual public IActor
{
public:
    virtual void setThrottle(float)=0;
    virtual float getThrottle()=0;
};

class IProjectile: virtual public IActor
{
public:
    virtual void shoot(
        const Vector &pos, const Vector &vec, const Vector &dir)=0;
};

class IFollower : virtual public Object
{
public:
    virtual void follow(Ptr<IActor>)=0;
};

class IFontMan : virtual public Object
{
public:
    struct FontSpec {
        enum Style { STANDARD, BOLD };
        FontSpec(const std::string & name, int size = 10, Style style = STANDARD);

        std::string name;
        int size;
        Style style;
    };
    virtual void selectFont(const FontSpec & font)=0;
    virtual void setColor(const Vector & col)=0;
    virtual void setAlpha(float)=0;
    virtual void setCursor(const Vector & c,
                   const Vector & px,
                   const Vector & py)=0;
    virtual const Vector & getCursor()=0;
    virtual void print(const char *text)=0;
};

class Model;
class IModelMan : virtual public Object
{
public:
    virtual Ptr<Model> query(const std::string & name)=0;
    virtual void flush()=0;
};

#endif
