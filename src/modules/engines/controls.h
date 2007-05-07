#ifndef CONTROLS_H
#define CONTROLS_H

#include <string>
#include <map>

#include <landscape.h>
#include <DataNode.h>
#include <modules/scripting/IoIncludes.h>

struct IGame;

class FlightControls : virtual public Object {
protected:
    Ptr<DataNode> c;
public:
    FlightControls() {
        c = new DataNode;
        setElevator(0);
        setAileronAndRudder(0);
        setThrottle(0);
    }

    FlightControls(Ptr<DataNode> c) : c(c)
    {
        setElevator(0);
        setAileronAndRudder(0);
        setThrottle(0);
    }

    void setElevator(float val) { c->setFloat("elevator", val); }
    float getElevator() const { return c->getFloat("elevator"); }
    
    void setAileronAndRudder(float val) {
        c->setFloat("aileron", val);
        c->setFloat("rudder", val);
    }
    void setAileron(float val) { c->setFloat("aileron", val); }
    void setRudder(float val) { c->setFloat("rudder", val); }
    float getAileron() const { return c->getFloat("aileron"); }
    float getRudder() const { return c->getFloat("rudder"); }
    
    void setThrottle(float val) { c->setFloat("throttle", val); }
    float getThrottle() const { return c->getFloat("throttle"); }

    Ptr<DataNode> getDataNode() { return c; }
};

class CarControls : virtual public SigObject {
protected:
    Ptr<DataNode> c;
public:
    CarControls() {
        c = new DataNode;
        setSteer(0);
        setThrottle(0);
        setBrake(0);
    }

    CarControls(Ptr<DataNode> c) : c(c)
    {
        setSteer(0);
        setThrottle(0);
        setBrake(0);
    }
    
    void  setSteer(float val) { c->setFloat("steer", val); }
    float getSteer() { return c->getFloat("steer"); }
    
    void  setThrottle(float val) { c->setFloat("throttle", val); }
    float getThrottle() { return c->getFloat("throttle"); }

    void  setBrake(float val) { c->setFloat("brake", val); }
    float getBrake() { return c->getFloat("brake"); }
    
    Ptr<DataNode> getDataNode() { return c; }
};

class TankControls : virtual public CarControls {
public:
    TankControls()
    {
        setTurretSteer(0);
        setCannonSteer(0);
    }

    TankControls(Ptr<DataNode> c)
        : CarControls(c)
    {
        setTurretSteer(0);
        setCannonSteer(0);
    }

    void  setTurretSteer(float val) { c->setFloat("turret_steer", val); }
    float getTurretSteer() { return c->getFloat("turret_steer"); }

    void  setCannonSteer(float val) { c->setFloat("cannon_steer", val); }
    float getCannonSteer() { return c->getFloat("cannon_steer"); }
};

#endif
