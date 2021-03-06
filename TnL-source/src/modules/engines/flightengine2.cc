#include "flightengine2.h"

#include <tnl.h>
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
        return out;
    }
}

FlightEngine2::FlightEngine2(Ptr<IGame> game,
        float max_thrust,
        float wing_area,
        float max_torque)
: RigidEngine(game),
  max_thrust(max_thrust), max_torque(max_torque),
  wing_area(wing_area)
{
    construct(2000, 200000, 160000, 100000);
    clearForces();
}


void FlightEngine2::setControls(Ptr<DataNode> controls) {
    this->controls = new FlightControls(controls);
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
    if (direction*front < 0) {
        alpha = PI - alpha;
        alpha_h = PI - alpha_h;
    }

    //const static float CL_x[4] = {-8.0, 0.0, 18.0, 32.0};
    //const static float CL_y[4] = {-0.2, 0.5,  1.6,  1.0};
    const static float CL_x[] = {-180.0, -135.0, -90.0, -45.0, -8.0, 0.0, 18.0, 32.0, 90.0, 135.0, 180.0};
    const static float CL_y[] = {  -0.5,    1.6,   0.0,  -1.0, -0.2, 0.2,  1.6,  1.0,  0.0,  -1.6,  -0.5};
    //const static float CD_x[] = {-8.0, 0.0, 8.0, 16.0, 24.0};
    //const static float CD_y[] = {0.01,.015, .05,  0.7,  1.5};
    const static float CD_x[] = {-180.0, -90.0, -45.0, -24.0, -16.0, -8.0,  0.0, 8.0,  16.0, 24.0, 45.0, 90.0, 180.0};
    const static float CD_y[] = {   1.5,   3.0,   2.0,   0.8,   0.2,  .18, 0.15, .18,   0.2,  0.8,  3.5,  4.0,   1.5};
    //const static float CD_h_x[] = {-8.0, 0.0, 8.0};
    //const static float CD_h_y[] = {-1.0, 0.0, 1.0};
    const static float CD_h_x[] = {-180.0, -172.0, -120.0, -90.0, -60.0, -8.0, 0.0, 8.0, 60.0, 90.0, 120.0, 172.0, 180.0};
    const static float CD_h_y[] = {   0.0,   -1.0,   -2.5,  -3.0,  -2.5, -1.0, 0.0, 1.0,  2.5,  3.0,   2.5,   1.0,   0.0};
    const static float C_torque_x[] = { -180, -120, -90, -60, -10,  0, 10, 60, 90, 120, 180};
    const static float C_torque_y[] = {    0,  -.5,  -1, -.5, -.1,  0, .1, .5,  1,  .5,   0};
    const static int nCL = sizeof(CL_x)/4;
    const static int nCD = sizeof(CD_x)/4;
    const static int nCD_h = sizeof(CD_h_x)/4;
    const static int nC_torque = sizeof(C_torque_x)/4;

    float C_L =   interp(nCL, alpha * 180 / PI, CL_x, CL_y);
    float C_D =   interp(nCD, alpha * 180 / PI, CD_x, CD_y);
    float C_D_h = interp(nCD_h, alpha_h * 180 / PI, CD_h_x, CD_h_y);

    float V2 = v.lengthSquare();
    float V = sqrt(V2);
    
    float Vxz2 = (v - up*(v*up)).lengthSquare();
    float Vxz = sqrt(Vxz2);
    
    float Vyz2 = (v - right*(v*right)).lengthSquare();
    float Vyz = sqrt(Vyz2);
    
    float Vx = fabs(v * right);
    float Vx2 = Vx*Vx;

    float Vz = fabs(v * front);
    float Vz2 = Vz*Vz;
    
    float L = 0.5 * C_L * rho * Vyz2 * wing_area;
    float D = 0.5 * C_D * rho * Vyz2 * wing_area;
    float D_h = 0.5 * C_D_h * rho * Vx2 * wing_area;
    
    Vector aero_force = up*L - direction * D - right * D_h;
    float delta_Ekin = aero_force * v;
    if (delta_Ekin > 0) {
        ls_error("delta Ekin: %5.5f\n", delta_Ekin);
    }

    applyForce((max_thrust * controls->getThrottle()) * front);
    applyForce(up * L);
    applyForce(direction * -D);
    applyForce(right * -D_h);
    applyLinearAcceleration(Vector(0,-9.81,0));
    
    float C_torque = interp(nC_torque, alpha * 180/PI, C_torque_x, C_torque_y);
    float torque = 0.5 * C_torque * rho * wing_area * Vyz;
    float C_torque_h = interp(nC_torque, alpha_h * 180/PI, C_torque_x, C_torque_y);
    float torque_h = 0.5 * C_torque_h * rho * wing_area * Vxz;
    
    applyTorque(200 * right * torque);
    applyTorque(5000 * up * torque_h);
    applyTorque(0.01 * Vz * front * (max_torque * -controls->getAileron()));
    applyTorque(0.01 * Vz * right * (max_torque * controls->getElevator()));
    applyTorque(0.01 * Vz * up    * (max_torque * controls->getRudder()));

    Vector omega = getAngularVelocity();
    Vector omega_xy = front * (front * omega);
    Vector omega_rest = omega - omega_xy;
    //applyAngularAcceleration(-0.001*V2 * omega * omega.length());
    applyAngularAcceleration(-0.001*V2 * omega_rest * omega_rest.length());
    applyAngularAcceleration(-0.0005*V2 * omega_xy * omega_xy.length());

    RigidEngine::run();
}

