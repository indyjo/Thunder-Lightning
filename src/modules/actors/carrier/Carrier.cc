#include <string>
#include <interfaces/IConfig.h>
#include <interfaces/IModelMan.h>
#include <modules/collide/CollisionManager.h>
#include <modules/engines/effectors.h>
#include "Carrier.h"


Carrier::Carrier(Ptr<IGame> thegame, IoObject * io_peer_init)
: SimpleActor(thegame)
{
    if (io_peer_init)
        setIoObject(io_peer_init);
    else {
        //createIoObject();
    }
    
    setTargetInfo(new TargetInfo(
        "Carrier", 78, TargetInfo::CARRIER));
        
    this->engine = new RigidEngine(thegame);
    float m = thegame->getConfig()->queryFloat("Carrier_mass", 50000000.0f); // mass
    float w = 2*30.96f;    // width, height and depth taken from the model
    float h = 2*14.833f;
    float d = 2*76.524f;
    float f = m / 12;
    // construct intertia like cuboid with these dimensions
    engine->construct(m, f*(h*h+d*d), f*(w*w+d*d), f*(w*w+h*h));
    engine->addEffector(Effectors::Gravity::getInstance());
    setEngine(engine);
    
    std::string skeletonfile = thegame->getConfig()->query("Carrier_skeleton");
    setSkeleton(new Skeleton(thegame, skeletonfile));

    Ptr<Model> hull_model = thegame->getModelMan()->query(thegame->getConfig()->query("Carrier_model_hull"));
    Effectors::Buoyancy::addBuoyancyFromMesh(
        engine,
        hull_model->getDefaultObject(),
        Vector(0,0,0));
        
    
    // Prepare collidable
    setBoundingGeometry(
        thegame->getCollisionMan()->queryGeometry(
            thegame->getConfig()->query("Carrier_model_bounds")));
    setRigidBody(ptr(engine));
    setActor(this);
    
    Transform xform(
        Quaternion::Rotation(Vector(-1,0,0), 70*3.141593f/180), 
        Vector(0,0,0));
    skeleton->setBoneTransform("BayDoor", xform);
}

void Carrier::onLinked() {
    SimpleActor::onLinked();
    thegame->getCollisionMan()->add(this);
}

void Carrier::onUnlinked() {
    SimpleActor::onUnlinked();
    thegame->getCollisionMan()->remove(this);
}

void Carrier::integrate(float delta_t, Transform * transforms) {
    engine->integrate(delta_t, transforms);
    transforms[1] = skeleton->getEffectiveBoneTransform("Deck");
    transforms[2] = skeleton->getEffectiveBoneTransform("DeckTower");
    transforms[3] = skeleton->getEffectiveBoneTransform("Radom");
    transforms[4] = skeleton->getEffectiveBoneTransform("DockingBay");
    transforms[5] = skeleton->getEffectiveBoneTransform("BayDoor");
}


void Carrier::update(float delta_t, const Transform * new_transforms) {
    engine->update(delta_t, new_transforms);
}

