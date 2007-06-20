#include <modules/clock/clock.h>
#include <interfaces/ITerrain.h>

#include "carengine.h"


CarEngine::CarEngine(Ptr<IGame> game,
    const CarParams & params)
    : thegame(game),
      CarParams(params)
{
    //ls_message("CarEngine::Carengine. thegame->getTerrain()=%p\n", &*thegame->getTerrain());
    terrain = thegame->getTerrain();
    p = Vector(0,0,0);
    v = 0.0f;
    front = Vector(0,0,1);
    update();
}

void CarEngine::setControls(Ptr<DataNode> controls) {
    this->controls = new CarControls(controls);
}

void CarEngine::run()
{
    float delta_t = thegame->getClock()->getStepDelta();

    float force = controls->getThrottle() * max_accel;
    force -= controls->getBrake() * max_brake;
    force -= v * std::abs(v) * drag_coeff;
    force -= v * res_coeff;

    //ls_message("force: %f speed: %f\n", force, v);

    v += delta_t * force / mass;

    float rot = (controls->getSteer() * v * delta_t) * max_rot_per_meter;
    front *= RotateYMatrix<float>(rot);

    p += delta_t*(front*v);

    update();
}

Vector CarEngine::getLocation()    { return p; }
Vector CarEngine::getFrontVector() { return front; }
Vector CarEngine::getRightVector() { return right; }
Vector CarEngine::getUpVector()    { return up; }
void CarEngine::getOrientation(Vector * up, Vector * right, Vector * front)
{
    *up = this->up;
    *right = this->right;
    *front = this->front;
}

Vector CarEngine::getMovementVector() { return v*front; }

void CarEngine::setLocation(const Vector & pos) {
    p = pos;
    update();
}

void CarEngine::setOrientation(const Vector & up,
                               const Vector & right,
                               const Vector & front)
{
    this->front = front;
    update();
}

void CarEngine::setMovementVector(const Vector & vel) {
    v = front*vel;
}

void CarEngine::update() {
    Vector2 f(front[0],front[2]);
    Vector2 r(f[1], -f[0]);
    Vector rotated_tripod[3];
    for(int i=0; i<3; i++) {
        Vector2 x = Vector2(p[0],p[2]) + r*tripod[i][0] + f*tripod[i][2];
        rotated_tripod[i] = Vector(
            x[0],
            terrain->getHeightAt(x[0], x[1]),
            x[1]);
        // Cheap-ass buoyancy simulation, i.e. a tank/car/whatever will stay
        // over water with 1m penetration
        if (rotated_tripod[i][1] < -1) {
            rotated_tripod[i][1] = -1;
        }
    }
    up = (rotated_tripod[1] - rotated_tripod[0]) % (rotated_tripod[2] - rotated_tripod[0]);
    up.normalize();
    right = up % front;
    right.normalize();
    front = right % up;
    front.normalize();

    p[1] = std::max(-1.0f, terrain->getHeightAt(p[0], p[2]));
}
