#ifndef CONTROLS_H
#define CONTROLS_H

#include <string>
#include <map>

#include <landscape.h>

#include <modules/scripting/IoIncludes.h>

struct IGame;

class Controls : virtual public Object {
    IoObject *self;
public:
    class NoSuchControl { };
    
    Controls(IGame *thegame);
    Controls(IoState *state);
    ~Controls();

    inline IoObject *getObject() { return self; }
    bool getBool(const std::string & name, bool default_value);
    bool getBool(const std::string & name) const throw(NoSuchControl);
    void setBool(const std::string & name, bool val);
    
    int getInt(const std::string & name, int default_value);
    int getInt(const std::string & name) const throw(NoSuchControl);
    void setInt(const std::string & name, int val);

    float getFloat(const std::string & name, float default_value);
    float getFloat(const std::string & name) const throw(NoSuchControl);
    void setFloat(const std::string & name, float val);

    Vector getVector(const std::string & name, Vector default_value);
    Vector getVector(const std::string & name) const throw(NoSuchControl);
    void setVector(const std::string & name, Vector val);
};

class FlightControls : virtual public Object {
protected:
    Ptr<Controls> c;
public:
    FlightControls(IGame *thegame) {
        c = new Controls(thegame);
        setElevator(0);
        setAileronAndRudder(0);
        setThrottle(0);
    }

    FlightControls(Ptr<Controls> c) : c(c)
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

    Ptr<Controls> getControls() { return c; }
};

class CarControls : virtual public SigObject {
protected:
    Ptr<Controls> c;
public:
    CarControls(IGame *thegame) {
        c = new Controls(thegame);
        setSteer(0);
        setThrottle(0);
        setBrake(0);
    }

    CarControls(Ptr<Controls> c) : c(c)
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
    
    Ptr<Controls> getControls() { return c; }
};

class TankControls : virtual public CarControls {
public:
    TankControls(IGame *thegame)
        :CarControls(thegame)
    {
        setTurretSteer(0);
        setCannonSteer(0);
    }

    TankControls(Ptr<Controls> c)
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
