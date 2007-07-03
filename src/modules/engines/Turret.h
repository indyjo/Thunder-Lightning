#ifndef TNL_TURRET_H
#define TNL_TURRET_H

#include <string>
#include <vector>
#include <modules/math/Transform.h>
#include <tnl.h>

/// Abstracts a kinematic chain element of a turret. Every turret has two
/// elements: base and elevator. Every element rotates around some specified
/// axis. There is a minimum and a maximum rotation angle. The minimum angle must
/// lie in [-PI;0] and the maximum angle in [0;PI]. These angles only apply if
/// is_restriced is true; otherwise the element's rotation is unrestricted.
/// A reference vector defines the direction of zero rotation.
struct TurretElement {
    Vector axis;                //< The element's rotation axis
    Vector reference;           //< Some vector orthogonal to axis which defines the 
    float min_angle, max_angle; //< min and max constraints
    bool is_restricted;         //< whether min_angle and max_angle apply
    float max_rotation_speed;   //< maximum speed in radians per second with which the element rotates
};

/// A class representing a turret with two kinematic elements.
/// Control, graphics, collision detection and weapon firing must be provided
/// from outside.
class Turret: public Object {
    // Base variables ----------------------------------------------------------
    TurretElement elements[2];
    Vector pivot;
    
    // State variables ---------------------------------------------------------
    float angles[2];
    
public:
    /// Constructs a turret.
    /// @param base     The base element
    /// @param elevator The elevator element
    /// @param pivot    A point in the crossing of the two elements' respective axes,
    ///                 in the actor's local coordinate system
    Turret(const TurretElement & base, const TurretElement & elevator, const Vector & pivot);
    
    ~Turret();
    
    /// Integrates element state in time using control input.
    Transform integrate(int element_idx, float delta_t, float control);
    /// Updates element state to the transform provided. Ignores the
    /// transform's translational component.
    void update(int element_idx, const Transform &);
    
    /// Return current angle of element with given index
    inline float getAngle(int element_idx) { return angles[element_idx]; }
};

#endif

