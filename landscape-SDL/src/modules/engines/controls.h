#ifndef CONTROLS_H
#define CONTROLS_H

#include <string>
#include <map>

#include <landscape.h>

class Controls : virtual public Object {
    std::map<std::string, bool>  bools;
    std::map<std::string, int>   ints;
    std::map<std::string, float> floats;
public:
    class NoSuchControl { };
    
    Controls() { }
    bool getBool(const std::string & name) const throw(NoSuchControl);
    void setBool(const std::string & name, bool val);
    int getInt(const std::string & name) const throw(NoSuchControl);
    void setInt(const std::string & name, int val);
    float getFloat(const std::string & name) const throw(NoSuchControl);
    void setFloat(const std::string & name, float val);
};

class FlightControls : virtual public Object {
protected:
    Ptr<Controls> c;
public:
    FlightControls(Ptr<Controls> c = new Controls()) : c(c)
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
    CarControls(Ptr<Controls> c = new Controls()) : c(c)
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
    TankControls(Ptr<Controls> c = new Controls)
        : CarControls(c)
    {
        setTurretSteer(0);
        setCannonSteer(0);
        setFire(false);
        setReload(false);
        setAmmoType(0);
    }

    void  setTurretSteer(float val) { c->setFloat("turret-steer", val); }
    float getTurretSteer() { return c->getFloat("turret-steer"); }

    void  setCannonSteer(float val) { c->setFloat("cannon-steer", val); }
    float getCannonSteer() { return c->getFloat("cannon-steer"); }

    void  setFire(bool val) { c->setBool("fire", val); }
    bool  getFire() { return c->getBool("fire"); }

    void  setReload(bool val) { c->setBool("reload", val); }
    bool  getReload() { return c->getBool("reload"); }

    void  setAmmoType(int val) { c->setInt("ammo-type", val); }
    int   getAmmoType() { return c->getInt("ammo-type"); }
};

class DroneControls : virtual public FlightControls {
public:
    DroneControls(Ptr<Controls> c = new Controls)
    :   FlightControls(c)
    {
        setPrimary(0);
        setSecondary(0);
        setFirePrimary(false);
        setFireSecondary(false);
    }

    void setPrimary(int val) { c->setInt("primary", val); }
    int  getPrimary() { return c->getInt("primary"); }
    void setSecondary(int val) { c->setInt("secondary", val); }
    int  getSecondary() { return c->getInt("secondary"); }

    void setFirePrimary(bool val) { c->setBool("fire-primary", val); }
    bool getFirePrimary() { return  c->getBool("fire-primary"); }

    void setFireSecondary(bool val) { c->setBool("fire-secondary", val); }
    bool getFireSecondary() { return  c->getBool("fire-secondary"); }
};

#endif
