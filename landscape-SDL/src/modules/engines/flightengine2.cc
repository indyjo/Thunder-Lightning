#include "flightengine2.h"

#include <landscape.h>
#include <modules/clock/clock.h>

namespace {
    float interp(int n, float t, const float x[], const float y[]) {
        int idx=-1;
        for(int i=0; i<n-1; i++) {
            if(x[i] < t && t <= x[i+1]) {
                idx = i;
                break;
            }
        }
        if (idx==-1) {
            if (t <= x[0]) idx = 0;
            else idx = n-2;
        }

        return y[idx] + ((t-x[idx]) / (x[idx+1]-x[idx])) * (y[idx+1] - y[idx]);
    }

    std::ostream & operator << (std::ostream & out, const Vector & v) {
        out << '(' << v[0] << ", " << v[1] << ", " << v[2] << ')';
        return out;
    }
    std::ostream & operator << (std::ostream & out, const Matrix & m) {
        out << '(' << std::endl;
        for (int i=0; i<4; i++) {
            for (int j=0; j<4; j++)
                out << m(i,j) << ", ";
            out << std::endl;
        }
        out << ')';
    }
}

FlightEngine2::FlightEngine2(Ptr<IGame> game,
        float max_thrust,
        float wing_area,
        float max_torque)
: RigidEngine(game),
  max_thrust(max_thrust), max_torque(max_torque),
  wing_area(wing_area),
  controls(new FlightControls)
{
    construct(2000, 200000, 250000, 160000);
    clearForces();
}


#define PI 3.14159265358979323846

// Air pressure
#define rho 1.293

void FlightEngine2::run() {
    float delta_t = thegame->getClock()->getStepDelta();

    Vector up, right, front;
    getOrientation(&up, &right, &front);

    Vector v = getLinearVelocity();
    Vector direction = Vector(v).normalize();
    if (v.lengthSquare() < 1e-5) direction = front;

    // Angle of attack (vertical, horizontal)
    float alpha   = -asin(direction * up);
    float alpha_h =  asin(direction * right);

    const static float CL_x[4] = {-8.0, 0.0, 18.0, 32.0};
    const static float CL_y[4] = {-0.2, 0.5,  1.6,  1.0};
    const static float CD_x[5] = {-8.0, 0.0, 8.0, 16.0, 24.0};
    const static float CD_y[5] = {0.01,.015, .05,  0.7,  1.5};
    const static float CD_h_x[3] = {-8.0, 0.0, 8.0};
    const static float CD_h_y[3] = {-1.0, 0.0, 1.0};

    float C_L =   interp(4, alpha * 180 / PI, CL_x, CL_y);
    float C_D =   interp(5, alpha * 180 / PI, CD_x, CD_y);
    float C_D_h = interp(3, alpha_h * 180 / PI, CD_h_x, CD_h_y);

    float V2 = v.lengthSquare();
    float L = 0.5 * C_L * rho * V2 * wing_area;
    float D = 0.5 * C_D * rho * V2 * wing_area;
    float D_h = 0.5 * C_D_h * rho * V2 * wing_area;

    applyForce((max_thrust * controls->getThrottle()) * front);
    applyForce(up * L);
    applyForce(direction * -D);
    applyForce(right * -D_h);
    applyLinearAcceleration(Vector(0,-9.81,0));

    applyTorque(front * (max_torque * -controls->getAileron()));
    applyTorque(right * (max_torque * controls->getElevator()));
    applyTorque(up    * (max_torque * controls->getRudder()));

    const Vector & omega = getAngularVelocity();
    applyAngularAcceleration(-2.5f * omega * omega.length());

    RigidEngine::run();
}

