#ifndef TANKENGINE_H
#define TANKENGINE_H

#include <vector>
#include "carengine.h"

struct TankParams {
    float turret_rot_speed;
    float cannon_rot_speed;
    float min_cannon_rot, max_cannon_rot;
    float reload_time;

    TankParams(
        float trs = 1.0f,
        float crs = 0.3f,
        float mincr = -0.14f,
        //float maxcr = 0.19f,
        float maxcr = 1.0f,
        //float rt = 5.0f)
        float rt = 0.1f)
    :
        turret_rot_speed(trs),
        cannon_rot_speed(crs),
        min_cannon_rot(mincr),
        max_cannon_rot(maxcr),
        reload_time(rt)
    { }
};

#define CANNON_READY   0
#define CANNON_LOADING 1

class TankEngine: virtual public CarEngine,
                  protected TankParams {
public:
    TankEngine(Ptr<IGame> game,
               Ptr<TankControls> ctrls,
               const CarParams & cparams = CarParams(),
               const TankParams & params = TankParams());

    // IEngine
    virtual void run();

    inline ActionSignal & getFireSignal() { return fire_sig; }

    inline float getTurretAngle() { return turret_angle; }
    inline float getCannonAngle() { return cannon_angle; }
    inline void reloadCannon() {
        if (!isCannonReady()) return;
        cannon_state = CANNON_LOADING;
        reload_time_left = reload_time;
    }
    inline bool isCannonLoading() { return cannon_state == CANNON_LOADING; }
    inline bool isCannonReady()   { return cannon_state == CANNON_READY; }
    inline int  getAmmoType()     { return ammo_type; }
    inline void setAmmoType(int t) {
        ammo_type = t;
        reloadCannon();
    }
    inline int  getMaxAmmoType()  { return ammo.size(); }
    inline int  getAmmo(int type) { return ammo[type]; }
    inline void setAmmo(int type, int n) {
        if (type >= ammo.size()) ammo.resize(type+1);
        ammo[type] = n;
    }
    inline Vector getRelativeCannonVector() {
        Vector v(0,0,1);
        v =  RotateYMatrix<float>(turret_angle)
             * (RotateXMatrix<float>(-cannon_angle) * v);
        return v;
    }


protected:
    Ptr<TankControls> tank_controls;
    float turret_angle, cannon_angle;
    int cannon_state;
    int ammo_type;
    std::vector<int> ammo;
    ActionSignal fire_sig;
    float reload_time_left;
};

#endif
