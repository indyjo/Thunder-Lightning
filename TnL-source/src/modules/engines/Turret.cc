#include <cmath>
#include "Turret.h"

Turret::Turret(const TurretElement & base, const TurretElement & elevator, const Vector & pivot)
    : pivot(pivot)
{
    elements[0] = base;
    elements[1] = elevator;
    angles[0] = 0;
    angles[1] = 0;
}

Turret::~Turret()
{ }

#define PI 3.14159265358979323846

Transform Turret::integrate(int element_idx, float delta_t, float control)
{
    TurretElement &element = elements[element_idx];
    float angle = angles[element_idx];
    
    if (delta_t == 0) {
        return Transform(Quaternion::Rotation(element.axis, angle), Vector(0));
    }
    
    float c = std::max(-1.0f, std::min(1.0f, control));
    float new_angle = angle + delta_t * element.max_rotation_speed * c;
    if (element.is_restricted) {
        new_angle = std::max(element.min_angle, std::min(element.max_angle, new_angle));
    } else {
        while (new_angle < -PI) new_angle += 2*PI;
        while (new_angle >  PI) new_angle -= 2*PI;
    }
    
    return Transform(Quaternion::Rotation(element.axis, new_angle), Vector(0));
}


void Turret::update(int element_idx, const Transform & xform) {
    TurretElement &element = elements[element_idx];
    
    // real part of quaternion is cos(angle/2)
    // imaginary part of quaternion is axis * sin(angle/2)
    float new_angle = 2*acos(xform.quat().real());
    float sign = element.axis * xform.quat().imag();
    if ( sign < 0 ) new_angle = - new_angle;

    // new_angle is already between -PI and PI, but we have to enforce constraints
    if (element.is_restricted) {
        new_angle = std::max(element.min_angle, std::min(element.max_angle, new_angle));
    }
    
    angles[element_idx] = new_angle;
}

