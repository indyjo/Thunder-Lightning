#include <landscape.h>
#include <modules/math/Vector.h>
#include <modules/math/Matrix.h>
#include <modules/actors/fx/spark.h>
#include <modules/actors/simpleactor.h>
#include <modules/collide/CollisionManager.h>
#include <modules/engines/rigidengine.h>

#define BULLET_TRAIL_BUFFER 3

class Bullet: public IProjectile, public SimpleActor, public Collide::Collidable
{
public:
    static int no_collide_tag;

    Bullet(IGame *thegame, Ptr<IActor> source=0, float factor=1);

    virtual void action();

    virtual void draw();

    virtual void shoot(const Vector &pos, const Vector &vec, const Vector &dir);
    virtual Ptr<IActor> getSource();
    
    inline void setTTL(double ttl) { this->ttl = ttl; }

    virtual void integrate(float delta_t, Transform * transforms);
    virtual void update(float delta_t, const Transform * new_transforms);
    virtual void collide(const Collide::Contact & c);

private:
    void die();
    void explode();

private:
    JRenderer *renderer;
    Ptr<ITerrain> terrain;
    Ptr<IPositionProvider> camera;
    Ptr<RigidEngine> engine;
    Ptr<IActor> source;
    float age, ttl, factor;
};
