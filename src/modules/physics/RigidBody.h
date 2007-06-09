#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <tnl.h>
#include <modules/math/Quaternion.h>


struct RigidBodyState {
    Vector     x;          // Position in world coordinates
    Quaternion q;          // Orientation
    Vector     P;          // Linear momentum  (Impuls in German)
    Vector     L;          // Angular momentum (Drehimpuls in German)
};

struct RigidBodyBase {
    float   M;          // The rigid body's mass
    Matrix3 I;          // Inertia matrix in object coordinates
    float   M_inv;      // 1/M
    Matrix3 I_inv;      // Inverse inertia matrix in object coordinates
};


class RigidBody : private RigidBodyBase,
                  private RigidBodyState,
                  virtual public Object
{
    // Variables derived from M, I and the current state
    Quaternion q_inv;      // Inverse Orientation (q.conj())
    Matrix3    I_wcs;      // The inertia matrix in world coordinates
    Matrix3    I_inv_wcs;  // Inverse inertia matrix in world coordinates
    Matrix3    R, R_inv;   // Orientation matrix and its inverse
    Vector     v, omega;   // Linear and angular velocity

    // Variables which are held for exactly one computation interval
    Vector  F;          // Linear force
    Vector  torque;     // (Angular) torque (in German: Drehmoment)

public:

    RigidBody();

    void construct(float M, float Ixx, float Iyy, float Izz);

    // Linear operations
    inline void applyLinearVelocity(const Vector & d_vel) {
        v += d_vel;
        P += M * d_vel;
    }
    inline void applyLinearImpulse(const Vector & impulse) {
        v += impulse * M_inv;
        P += impulse;
    }
    inline void applyForce(const Vector & force) {F += force;}
    inline void applyLinearAcceleration(const Vector & a) { F += M*a;}

    // Angular operations
    inline void applyAngularVelocity(const Vector & a_vel) {
        omega += a_vel;
        L += I_wcs * a_vel;
    }
    inline void applyAngularImpulse(const Vector & a_impulse) {
        omega += I_inv_wcs * a_impulse;
        L += a_impulse;
    }
    inline void applyTorque(const Vector & v) { torque += v; }
    inline void applyAngularAcceleration(const Vector & a) { torque += I_wcs * a; }

    inline void applyImpulseAt(const Vector & v, const Vector & r) {
        applyLinearImpulse(v);
        applyAngularImpulse((r-x) % v);
    }
    inline void applyVelocityAt(const Vector & v, const Vector & r) {
        applyLinearVelocity(v);
        applyAngularVelocity((r-x) % v);
    }
    inline void applyForceAt(const Vector & v, const Vector & r) {
        applyForce(v);
        applyTorque((r-x) % v);
    }
    inline void applyAccelerationAt(const Vector & v, const Vector & r) {
        applyLinearAcceleration(v);
        applyAngularAcceleration((r-x) % v);
    }

    // Returns the current velocity of a particle at position r in WCS
    inline Vector getVelocityAt(const Vector & r) const { return omega % (r - x) + v; }
    inline Vector getMomentumAt(const Vector & r) const { return L % (r - x) + P; }
    inline Vector getLinearVelocity() { return v; }
    inline Vector getAngularVelocity() { return omega; }

    inline const Vector & getLinearMomentum() const { return P; }
    inline void setLinearMomentum(const Vector & P_new) { P = P_new; v = P * M_inv; }

    inline const Vector & getAngularMomentum() const { return L; }
    inline void setAngularMomentum(const Vector & L_new) { L = L_new; omega = I_inv_wcs * L; }

    inline const RigidBodyState & getState() const { return *this; }
    inline void setState(const RigidBodyState & state) {
        (RigidBodyState &) *this = state;
        updateDerivedVariables();
    }

    inline const RigidBodyBase & getBase() const { return *this; }
    inline void setBase(const RigidBodyBase & base) {
        (RigidBodyBase &) *this = base;
        updateDerivedVariables();
    }

    inline void setStateAndBase(const RigidBodyState & state,
                                const RigidBodyBase & base)
    {
        (RigidBodyState &) *this = state;
        (RigidBodyBase &) *this = base;
        updateDerivedVariables();
    }

    RigidBodyState getDerivative() const;

    inline void clearForces() {
        F = Vector(0,0,0);
        torque = Vector(0,0,0);
    }

    /// Calculates the magnitude of the impulse to apply on RigidBody A in
    /// direction of Vector n and on RigidBody B in direction of -n if they
    /// collide in global point p.
    /// @param e Elasticity factor (1 meaning completely elastic collision,
    ///                             0 completely unelastic)
    /// @param A RigidBody A
    /// @param B RigidBody B
    /// @param p The point in global coordinate space where the collision happens
    /// @param n The normal at the point of collision (pointing towards A?)
    static float collisionImpulseMagnitude(
        float e,
        const RigidBody & A, const RigidBody & B,
        const Vector & p, const Vector & n);
    
    /// Calculates the magnitude of the impulse to apply on RigidBody A in
    /// direction of Vector n if it collides with an infinitely heavy body
    /// in global point p.
    /// @param e Elasticity factor (1 meaning completely elastic collision,
    ///                             0 completely unelastic)
    /// @param A RigidBody A
    /// @param p The point in global coordinate space where the collision happens
    /// @param n The normal at the point of collision (pointing towards A?)
    static float collisionImpulseMagnitude(
        float e,
        const RigidBody & A,
        const Vector & p, const Vector & n);

protected:
    void updateDerivedVariables();
};



#endif
