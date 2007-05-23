#include "RigidBody.h"

RigidBody::RigidBody() {
    clearForces();
    struct RigidBodyState state = {
        Vector(0,0,0),
        Quaternion(1, Vector(0,0,0)),
        Vector(0,0,0),
        Vector(0,0,0)};
    (RigidBodyState&) *this = state;
    construct(1,1,1,1);
}

void RigidBody::construct(float M, float Ixx, float Iyy, float Izz) {
    float I_base[] = {
        Ixx, 0, 0,
        0, Iyy, 0,
        0, 0, Izz};
    float I_inv_base[] = {
        1/Ixx, 0, 0,
        0, 1/Iyy, 0,
        0, 0, 1/Izz};
    struct RigidBodyBase new_base = {
        M,
        Matrix3::Array(I_base),
        1/M,
        Matrix3::Array(I_inv_base)};
    setBase(new_base);
}


RigidBodyState RigidBody::getDerivative() const {
    RigidBodyState derivative = {
        v,
        0.5f * (Quaternion(0, omega) * q),
        F,
        torque
    };
    return derivative;
}

void RigidBody::updateDerivedVariables() {
    q = q.normalize();
    q.toMatrix(R);
    R_inv = R;
    R_inv.transpose();
    I_wcs     = R * I * R_inv;
    I_inv_wcs = R * I_inv * R_inv;
    v = P * M_inv;
    omega = I_inv_wcs * L;
}


float RigidBody::collisionImpulseMagnitude(
        float e,
        const RigidBody & A, const RigidBody & B,
        const Vector & p, const Vector & n)
{
    const Matrix3 & A_I_inv = A.I_inv_wcs;
    const Matrix3 & B_I_inv = B.I_inv_wcs;
    const Vector & A_x = A.getState().x;
    const Vector & B_x = B.getState().x;
    Vector A_r = p - A_x;
    Vector B_r = p - B_x;

    float x1 = -(1+e)*((A.v-B.v)*n);
    float x2 = A.getBase().M_inv+B.getBase().M_inv;
    Vector x3 = (A_I_inv * (A_r % n)) % A_r;
    Vector x4 = (B_I_inv * (B_r % n)) % B_r;
    float x5 = n * (x3+x4);

    return x1 / (x2 + x5);
}

float RigidBody::collisionImpulseMagnitude(
        float e,
        const RigidBody & A,
        const Vector & p, const Vector & n)
{
    const Matrix3 & A_I_inv = A.I_inv_wcs;
    const Vector & A_x = A.getState().x;
    Vector A_r = p - A_x;

    float x1 = -(1+e)*(A.v*n);
    float x2 = A.getBase().M_inv;
    Vector x3 = (A_I_inv * (A_r % n)) % A_r;
    float x5 = n * x3;

    return x1 / (x2 + x5);
}

