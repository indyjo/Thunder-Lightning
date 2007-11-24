#ifndef TANKENGINE_H
#define TANKENGINE_H

#include <vector>
#include "rigidengine.h"
#include "controls.h"

struct TankParams {
    float turret_rot_speed;
    float cannon_rot_speed;
    float min_cannon_rot, max_cannon_rot;

    TankParams(
        float trs = 1.0f,
        float crs = 0.6f,
        float mincr = -0.14f,
        //float maxcr = 0.19f,
        float maxcr = 1.0f)
    :
        turret_rot_speed(trs),
        cannon_rot_speed(crs),
        min_cannon_rot(mincr),
        max_cannon_rot(maxcr)
    { }
};

class TankEngine: virtual public RigidEngine,
                  protected TankParams {
public:
    TankEngine(Ptr<IGame> game,
               const TankParams & params = TankParams());

    // IEngine
    virtual void setControls(Ptr<DataNode> controls);
    virtual void run();

    inline float getTurretAngle() { return turret_angle; }
    inline float getCannonAngle() { return cannon_angle; }
    inline Vector getRelativeCannonVector() {
        Vector v(0,0,1);
        v =  RotateYMatrix<float>(turret_angle)
             * (RotateXMatrix<float>(-cannon_angle) * v);
        return v;
    }


protected:
    Ptr<TankControls> tank_controls;
    float turret_angle, cannon_angle;
};

#endif
