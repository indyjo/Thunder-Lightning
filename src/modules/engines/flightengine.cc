#include "flightengine.h"

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
		return out;
    }
}

FlightEngine::FlightEngine(
    Ptr<IGame> game,
    float max_thrust, float wing_area, float mass,
    Vector p0, Vector v0, Matrix orient0)
: thegame(game),
  max_thrust(max_thrust), wing_area(wing_area), mass(mass),
  p(p0), v(v0), orient(orient0),
  roll_speed(0), pitch_speed(0), yaw_speed(0),
  controls(new FlightControls(ptr(game)))
{ }


#define PI 3.14159265358979323846

// Air pressure
#define rho 1.293

void FlightEngine::setControls(Ptr<Controls> controls) {
    this->controls = new FlightControls(controls);
}

void FlightEngine::run() {
    double delta_t = thegame->getClock()->getStepDelta();

//     cerr << "-------------------------------------------------------" << std::endl;
//     cerr << "object: " << (void *) this << std::endl;
//     cerr << "new frame time: " << delta_t << std::endl;

    Vector up, right, front;
    getOrientation(&up, &right, &front);

    Vector direction = Vector(v).normalize();

    Vector thrust = (max_thrust * controls->getThrottle()) * front;

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

    Vector lift = up * (L/mass);
    Vector drag = direction * (-D/mass);
    Vector drag_h = right * (-D_h/mass);

    Vector grav = Vector(0,-9.81,0);

    Vector force = thrust + drag + drag_h + lift + grav;

//     std::cerr << "p: " << p << std::endl;
//     std::cerr << "v: " << v << std::endl;
//     std::cerr << "orient: " << orient << std::endl;
//     std::cerr << "front: " << front << std::endl;
//     std::cerr << "right: " << right << std::endl;
//     std::cerr << "alpha: " << alpha << std::endl;
//     std::cerr << "C_D: " << C_D << std::endl;
//     std::cerr << "D: " << D << std::endl;
//     std::cerr << "alpha_h: " << alpha_h << std::endl;
//     std::cerr << "C_D_h: " << C_D_h << std::endl;
//     std::cerr << "D_h: " << D_h << std::endl;
//     std::cerr << "thrust: " << thrust << std::endl;
//     std::cerr << "drag: " << drag << std::endl;
//     std::cerr << "drag_h: " << drag_h << std::endl;
//     std::cerr << "lift: " << lift << std::endl;
//     std::cerr << "force: " << force << std::endl;

    p += delta_t * (v + 0.5*delta_t*force);
    v += delta_t * force;

    float roll_accel = 0.3 * controls->getAileron();
    float roll_drag = 0.1 * roll_speed * std::abs(roll_speed);
    roll_speed += roll_accel;
    roll_speed -= roll_drag;

    float pitch_accel = 0.15 * controls->getElevator();
    float pitch_drag = 0.4 * pitch_speed * std::abs(pitch_speed);
    pitch_speed += pitch_accel;
    pitch_speed -= pitch_drag;

    float yaw_accel = 0.1 * controls->getRudder();
    float yaw_drag  = 0.8 * yaw_speed * std::abs(yaw_speed);
    yaw_speed += yaw_accel;
    yaw_speed -= yaw_drag;

    float delta_roll  = - delta_t * roll_speed;
    float delta_pitch =   delta_t * pitch_speed;
    float delta_yaw   =   delta_t * yaw_speed;

    Matrix3 rot =   RotateZMatrix(delta_roll)
                  * RotateYMatrix(delta_yaw)
                  * RotateXMatrix(delta_pitch);

//     std::cerr << "aileron: " << controls->getAileron() << std::endl;
//     std::cerr << "roll_speed: " << roll_speed << std::endl;
//     std::cerr << "roll_accel: " << roll_accel << std::endl;
//     std::cerr << "roll_drag: " << roll_drag << std::endl;
//     std::cerr << "delta_roll: " << delta_roll << std::endl;
//     std::cerr << "delta_pitch: " << delta_pitch << std::endl;
//     std::cerr << "delta_yaw: " << delta_yaw << std::endl;
    orient = orient * Matrix::Hom(rot);
//     std::cerr << "orient after transform: " << orient << std::endl;
    orient.orthoNormalize();
}

// IPositionProvider
Vector FlightEngine::getLocation() { return p; }
Vector FlightEngine::getFrontVector() { return orient * Vector(0,0,1); }
Vector FlightEngine::getRightVector() { return orient * Vector(1,0,0); }
Vector FlightEngine::getUpVector()    { return orient * Vector(0,1,0); }
void FlightEngine::getOrientation(Vector *up, Vector *right, Vector *front) {
    *up = orient * Vector(0,1,0);
    *right = orient * Vector(1,0,0);
    *front = orient * Vector(0,0,1);
}

// IMovementProvider
Vector FlightEngine::getMovementVector() { return v; }

// IPositionReceiver
void FlightEngine::setLocation(const Vector & new_p) { p = new_p; }
void FlightEngine::setOrientation( const Vector & up,
                                   const Vector & right,
                                   const Vector & front) {
    orient = Matrix::Hom(MatrixFromColumns<float>(right, up, front));
}

// IMovementReceiver
void FlightEngine::setMovementVector(const Vector & new_v) { v = new_v; }

