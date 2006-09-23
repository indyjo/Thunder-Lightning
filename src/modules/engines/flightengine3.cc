#include "flightengine3.h"

#include <landscape.h>
#include <modules/clock/clock.h>

#define PI 3.14159265358979323846

FlightEngine3::FlightEngine3(Ptr<IGame> game,
        float max_thrust)
: RigidEngine(game),
  max_thrust(max_thrust)
{
    construct(2000, 250000, 250000, 160000);
    clearForces();

    AirFoil left_wing = {
        Vector(-3, 0, 0),
        Vector(0,0,1), Vector(0,1,0),
        8.875,
        0.01, 3.0,
        0.0, 0.1,
        Vector(-1,0,0),
        0,
        ""};
    AirFoil right_wing = {
        Vector(3, 0, 0),
        Vector(0,0,1), Vector(0,1,0),
        8.875,
        0.01, 3.0,
        0.0, 0.1,
        Vector(1,0,0),
        0,
        ""};
    AirFoil left_aileron = {
        Vector(-2.75, 0, -1.75),
        Vector(0,0,1), Vector(0,1,0),
        1.25,
        0.01, 3.0,
        0.0, 0.0,
        Vector(-1,0,0),
        45*PI/180,
        "aileron"};
    AirFoil right_aileron = {
        Vector(2.75, 0, -1.75),
        Vector(0,0,1), Vector(0,1,0),
        1.25,
        0.01, 3.0,
        0.0, 0.0,
        Vector(1,0,0),
        45*PI/180,
        "aileron"};
    AirFoil left_elevator = {
        Vector(-2.25, 0, -4),
        Vector(0,0,1), Vector(0,1,0),
        1.25,
        0.01, 5.0,
        0.0, 0.0,
        Vector(-1,0,0),
        45*PI/180,
        "elevator"};
    AirFoil right_elevator = {
        Vector(2.25, 0, -4),
        Vector(0,0,1), Vector(0,1,0),
        1.25,
        0.01, 5.0,
        0.0, 0.0,
        Vector(-1,0,0),
        45*PI/180,
        "elevator"};
    AirFoil rudder = {
        Vector(0, 1.25, -4),
        Vector(0,0,1), Vector(1,0,0),
        1.25,
        0.01, 5.0,
        0.0, 0.0,
        Vector(0,1,0),
        -10*PI/180,
        "rudder"};

    airfoils.push_back(left_wing);
    airfoils.push_back(right_wing);
    airfoils.push_back(left_aileron);
    airfoils.push_back(right_aileron);
    airfoils.push_back(left_elevator);
    airfoils.push_back(right_elevator);
    airfoils.push_back(rudder);
    return;
}

void FlightEngine3::setControls(Ptr<Controls> controls) {
    this->controls = new FlightControls(controls);
}

// Air pressure
#define rho 1.293

void FlightEngine3::run() {
    float delta_t = thegame->getClock()->getStepDelta();

    const Quaternion & q = getState().q;
    const Vector & x = getState().x;

    Vector up, right, front;
    getOrientation(&up, &right, &front);

    applyForce(front * max_thrust * controls->getThrottle());

    Ptr<Controls> c = controls->getControls();
    for(int i=0; i!=airfoils.size(); i++) {
        const AirFoil & foil = airfoils[i];

        float angle=0.0;
        if (foil.rot_param != "") angle = c->getFloat(foil.rot_param);
        angle *= foil.rot_factor;

        Quaternion q_foil = Quaternion::Rotation(foil.rot_axis, angle);
        q_foil = q * q_foil;

        Vector pos_foil = getState().x + q.rot(foil.position);
        Vector v = getVelocityAt(pos_foil);
        Vector direction = Vector(v).normalize();
        if (v.lengthSquare() < 1e-5) direction = front;

        Vector front_foil = q_foil.rot(foil.front);
        Vector up_foil = q_foil.rot(foil.up);

        float d_front = std::abs(direction * front_foil);
        float d_up = std::abs(direction * up_foil);

        float v_up = v * up_foil;
        float v_front = v * front_foil;

        float Cd = d_up * (foil.Cd_max-foil.Cd_min) + foil.Cd_min;
        float Cl = d_front * (foil.Cl_max-foil.Cl_min) + foil.Cl_min;

        Vector drag = - 0.5f * rho * foil.area * Cd * v * v.length();
        Vector lift = 0.5f * rho * foil.area * Cl * v.lengthSquare() * up_foil;

        drag = ((v_up*std::abs(v_up)) * up_foil * foil.Cd_max
               +(v_front*std::abs(v_front)) * front_foil * foil.Cd_min)
               * -0.5f * rho * foil.area;
        lift = ((v_up*std::abs(v_up)) * foil.Cl_min
               +(v_front*std::abs(v_front)) * foil.Cl_max) * up_foil
               * 0.5f * rho * foil.area;

        applyForceAt(lift+drag, pos_foil);
    }

    applyLinearAcceleration(Vector(0,-9.81,0));

    RigidEngine::run();
}

