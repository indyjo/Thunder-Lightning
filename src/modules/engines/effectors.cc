#include <modules/collide/CollisionManager.h>
#include <modules/engines/rigidengine.h>
#include <modules/model/model.h>
#include <TargetInfo.h>
#include "effectors.h"

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
}


namespace Effectors {

Gravity::Gravity() { }

void Gravity::applyEffect(RigidBody &rigid, Ptr<DataNode> controls) {
    rigid.applyLinearAcceleration(Vector(0,-9.81,0));
}

Ptr<Gravity> Gravity::singleton = 0;
Ptr<Gravity> Gravity::getInstance() {
    if (!singleton) singleton = new Gravity;
    return singleton;
}


#define PI 3.14159265358979323846

void Flight::applyEffect(RigidBody &rigid, Ptr<DataNode> cntrls) {
    // These are all variables involved in the flight simulation.
    // TODO: put these in a class FlightModel or similar
    const static float rho = 1.293; // air pressure
    const static float max_thrust = 40000.0;
    const static float wing_area = 20.0;
    const static float max_torque = 600000.0f;
    const static float torque_factor=200;
    const static float torque_h_factor=5000;
    const static float aileron_factor=-0.01;
    const static float elevator_factor=0.01;
    const static float rudder_factor=0.01;
    const static float Crot_rest=0.001;
    const static float Crot_xy=0.0005;
    const static float CL_x[] = {-180.0, -135.0, -90.0, -45.0, -8.0, 0.0, 18.0, 32.0, 90.0, 135.0, 180.0};
    const static float CL_y[] = {  -0.5,    1.6,   0.0,  -1.0, -0.2, 0.2,  1.6,  1.0,  0.0,  -1.6,  -0.5};
    const static float CD_x[] = {-180.0, -90.0, -45.0, -24.0, -16.0, -8.0,  0.0, 8.0,  16.0, 24.0, 45.0, 90.0, 180.0};
    const static float CD_y[] = {   0.75,   1.5,   1.0,   0.4,   0.2,  .18, 0.15, .18,  0.2,  0.4,  1.0,  1.5,   0.75};
    const static float CD_h_x[] = {-180.0, -172.0, -120.0, -90.0, -60.0, -8.0, 0.0, 8.0, 60.0, 90.0, 120.0, 172.0, 180.0};
    const static float CD_h_y[] = {   0.0,   -1.0,   -2.5,  -3.0,  -2.5, -1.0, 0.0, 1.0,  2.5,  3.0,   2.5,   1.0,   0.0};
    const static float C_torque_x[] = { -180, -120, -90, -60, -10,  0, 10, 60, 90, 120, 180};
    const static float C_torque_y[] = {    0,  -.5,  -1, -.5, -.1,  0, .1, .5,  1,  .5,   0};
    const static int nCL = sizeof(CL_x)/4;
    const static int nCD = sizeof(CD_x)/4;
    const static int nCD_h = sizeof(CD_h_x)/4;
    const static int nC_torque = sizeof(C_torque_x)/4;

    Quaternion q = rigid.getState().q;
    Vector up = q.rot(Vector(0,1,0));
    Vector right = q.rot(Vector(1,0,0));
    Vector front = q.rot(Vector(0,0,1));
    
    Vector v = rigid.getLinearVelocity();
    Vector direction = Vector(v).normalize();
    if (v.lengthSquare() < 1e-5) direction = front;

    // Angle of attack (vertical, horizontal)
    float alpha   = -asin(std::max(-1.0f, std::min(1.0f, direction * up)));
    float alpha_h =  asin(std::max(-1.0f, std::min(1.0f, direction * right)));
    if (direction*front < 0) {
        alpha = PI - alpha;
        alpha_h = PI - alpha_h;
    }

    float C_L =   interp(nCL, alpha * 180 / PI, CL_x, CL_y);
    C_L *= 1.0f - 0.3f*controls->getBrake();
    float C_D =   interp(nCD, alpha * 180 / PI, CD_x, CD_y);
    C_D *= 1.0f + 0.1f*controls->getBrake();
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
    
    Vector aero_force = (direction % right)*L - direction * D - right * D_h;
    float delta_Ekin = aero_force * v;
    if (delta_Ekin > 0) {
        ls_error("delta Ekin: %5.5f\n", delta_Ekin);
    }
    /*
    ls_message("v:"); v.dump();
    ls_message("dir:"); direction.dump();
    ls_message("right:"); right.dump();
    ls_message("front:"); front.dump();
    ls_message("right*dir: %.2f asin: %.2f degrees: %.2f\n", right*direction, asin(right*direction), asin(right*direction)*180/PI);
    ls_message("up:"); up.dump();
    ls_message("Lift: %.2f, drag: %.2f, drag_horiz: %.2f\n", L, D, D_h);
    ls_message("In v direction: Lift: %.2f, drag: %.2f, drag_horiz: %.2f\n",
        direction*(direction%right)*L, -D, -direction*right*D_h);
    */

    rigid.applyForce((max_thrust * controls->getThrottle()) * front);
    rigid.applyForce((direction % right) * L);
    rigid.applyForce(direction * -D);
    rigid.applyForce(right * -D_h);
    
    float C_torque = interp(nC_torque, alpha * 180/PI, C_torque_x, C_torque_y);
    float torque = 0.5 * C_torque * rho * wing_area * Vyz;
    float C_torque_h = interp(nC_torque, alpha_h * 180/PI, C_torque_x, C_torque_y);
    float torque_h = 0.5 * C_torque_h * rho * wing_area * Vxz;
    
    rigid.applyTorque(torque_factor * right * torque);
    rigid.applyTorque(torque_h_factor * up * torque_h);
    rigid.applyTorque(aileron_factor * Vz * front * (max_torque * controls->getAileron()));
    rigid.applyTorque(elevator_factor * Vz * right * (max_torque * controls->getElevator()));
    rigid.applyTorque(rudder_factor * Vz * up    * (max_torque * controls->getRudder()));

    Vector omega = rigid.getAngularVelocity();
    Vector omega_xy = front * (front * omega);
    Vector omega_rest = omega - omega_xy;

    rigid.applyAngularAcceleration(-Crot_rest*V2 * omega_rest * omega_rest.length());
    rigid.applyAngularAcceleration(-Crot_xy*V2 * omega_xy * omega_xy.length());
}




Wheel::Wheel(
    Ptr<ITerrain> terrain,
    Ptr<Collide::CollisionManager> cm,
    WeakPtr<Collide::Collidable> nocollide,
    const Params & params)
    : terrain(terrain), collision_manager(cm), nocollide(nocollide)
{ setParams(params); }

void Wheel::setParams(const Wheel::Params& p) {
    this->params = p;
    current_pos = Vector(0,0,0);
    current_load = 0;
    contact = false;
}

void Wheel::applyEffect(RigidBody &rigid, Ptr<DataNode> controls) {
    
    // the rigid body's orientation
    Quaternion q = rigid.getState().q;
    Vector up = q.rot(Vector(0,1,0));
    
    // wheel position in WCS
    Vector w = rigid.getState().x + q.rot(params.pos);
    
    // point of contact and respective normal vector
    Vector x, normal;
    
    // do the actual intersection test
    contact = terrain->lineCollides(w+params.range*up, w, &x, &normal);
    
    // Rigid body (if any) and velocity of collision partner
    Ptr<RigidBody> rigid_partner;
    Vector v_partner(0,0,0);
    
    if (!contact) {
        Ptr<Collide::Collidable> collidable =
            collision_manager->lineQuery(w+params.range*up, w, &x, &normal, nocollide.lock());
        if (collidable) {
            contact = true;
            Vector xloc = q.inv().rot(x - rigid.getState().x);
            Vector nloc = q.inv().rot(normal);
            rigid_partner = collidable->getRigid();
            if (rigid_partner) {
                v_partner = rigid_partner->getVelocityAt(x);
            }
        }
    }
    
    if (contact) {
        Vector right = q.rot(params.axis);
        Vector front = right % normal;
        front.normalize();
        right = normal % front;
        right.normalize();

        current_pos = x;
        // velocity of the wheel in WCS
        Vector v = rigid.getVelocityAt(x);
        
        // velocity of wheel in respect to collision partner
        Vector delta_v = v - v_partner;

        // relative load in interval [0..1]
        current_load = (x-w).length() / params.range;
        
        // Accumulated force to apply on rigid and (negatively) on partner rigid
        Vector force(0,0,0);
        
        // spring force along normal
        force += params.force * current_load * normal;
        // spring damping along up
        force += -params.damping  * up * (up*delta_v);
        // friction along front
        force += -params.drag_long * front*(front*delta_v);
        // sideways friction
        force += -params.drag_lat * right*(right*delta_v);
        
        rigid.applyForceAt(force, x);
        if (rigid_partner) {
            // Apply the force to the collision partner
            // FIXME: Beause of the integration scheme (every collidable
            //        for himself), these forces are currently ignored. :-(
            rigid_partner->applyForceAt(-force, x);
        }
    } else {
        current_pos = w;
    }
}

Thrust::Thrust()
    : max_force(Vector(0,0,0))
    , throttle(0)
{ }

void Thrust::applyEffect(RigidBody &rigid, Ptr<DataNode> controls) {
    rigid.applyForce(rigid.getState().q.rot(getEffectiveForce()));
}

void Missile::applyEffect(RigidBody &rigid, Ptr<DataNode> controls) {
    const static float cw_f = 0.3;              // frontal drag coefficient
    const static float cw_s = 1.2;              // side drag coefficient
    const static float torque_factor_z = 300;
    const static float torque_factor_xy = 300;
    const static float front_area = 3.141593f * 0.063f*0.063f;
    const static float side_area = 3.0f * 0.63f*0.63f;
    const static float rho= 1.293;            // air density

    Quaternion q=rigid.getState().q;
    Vector d = q.rot(Vector(0,0,1));
    Vector v = rigid.getLinearVelocity();

    // stabilizing torque
    Vector omega = rigid.getAngularVelocity();
    Vector omega_z = d * (omega*d);
    Vector omega_xy = omega - omega_z;
    Vector v_z = d * (v*d);
    Vector v_xy = v - v_z;
    rigid.applyTorque(-torque_factor_z * rho * omega_z.length()*omega_z);
    rigid.applyTorque(-torque_factor_xy * rho * omega_xy.length()*omega_xy);

    Vector v_f = d * (v*d); // frontal component of velocity
    Vector drag_force_f = -v_f.length() * v_f * (cw_f * front_area * rho / 2.0);

    Vector v_s = v - v_f; // side component of velocity
    Vector drag_force_s = -v_s.length() * v_s * (cw_s * side_area * rho / 2.0);

    rigid.applyForce(drag_force_f + drag_force_s);
}

void MissileControl::applyEffect(RigidBody &rigid, Ptr<DataNode> controls) {
    // control
    Vector a = controls->getVector("angular_accel");
    float length = a.length();
    // 10 is some random value to prevent unusually fast spinning
    if (length > 10.0f) {
        a *= 10/length;
    }
    rigid.applyAngularAcceleration(a);
}

void Buoyancy::addBuoyancyFromMesh(
    Ptr<RigidEngine> engine,
    Ptr<Model::Group> group,
    Ptr<Model::MeshData> mesh_data,
    Vector offset)
{
    typedef Model::Group::Faces::iterator Iter;
    for(Iter face = group->faces.begin(); face != group->faces.end(); ++face) {
        Vector a = mesh_data->vertices[ (*face)[0].v ];
        Vector b = mesh_data->vertices[ (*face)[1].v ];
        Vector c = mesh_data->vertices[ (*face)[2].v ];
        
        Vector n = (b-a)%(c-a);
        n.normalize();
        
        ls_message("a: "); a.dump();
        ls_message("b: "); b.dump();
        ls_message("c: "); c.dump();
        ls_message(" ->n: "); n.dump();
        
        Vector n2 = (b-a)%n;
        n2.normalize();
        
        // area is 0.5*base*height
        float area = std::abs(0.5f*(b-a).length() * (n2*(c-a)));
        
        engine->addEffector(new Buoyancy((a+b+c)/3 + offset, n, area));
    }
}

void Buoyancy::addBuoyancyFromMesh(
    Ptr<RigidEngine> engine,
    Ptr<Model::Object> obj,
    Vector offset)
{
    Ptr<Model::MeshData> mesh_data = obj->getMeshData();

    typedef std::vector<Ptr<Model::Group> > Groups;
    const Groups & groups = obj->getGroups();
    
    for(Groups::const_iterator i = groups.begin(); i != groups.end(); ++i) {
        addBuoyancyFromMesh(engine, *i, mesh_data, offset);
    }
}


void Buoyancy::applyEffect(RigidBody &rigid, Ptr<DataNode> controls) {
    const float rho = 1000.0f; // mass of water per cubic meter in kg
    const float g = 9.81f; // gravitation
    const float waterlevel = 0.0f;

    const RigidBodyState & state =rigid.getState();
    
    Vector p_wcs = state.x + state.q.rot(p);
    Vector n_wcs = state.q.rot(n);
    
    float depth = waterlevel - p_wcs[1];
    
    if (depth > 0) {
        float pressure = rho * g * depth; // in Pascal
        Vector buoyancy = - area * pressure * n_wcs;
        buoyancy[0] = buoyancy[2] = 0;
        rigid.applyForceAt(buoyancy, p_wcs);
        
        // drag
        const float C_d_normal = 1.2f;
        const float C_d_tangential = 0.02f;
        Vector v = rigid.getVelocityAt(p_wcs);
        Vector v_normal = n_wcs * (v * n_wcs);
        Vector v_tangential = v - v_normal;
        Vector drag_normal = -0.5f*rho*C_d_normal * area * v_normal.length() * v_normal;
        Vector drag_tangential = -0.5f*rho*C_d_tangential * area * v_tangential.length() * v_tangential;
        
        //ls_message("Applying normal drag: "); drag_normal.dump();
        //ls_message("     tangential drag: "); drag_tangential.dump();
        //ls_message("           against v: "); v.dump();
        
        rigid.applyForceAt(drag_normal, p_wcs);
        rigid.applyForceAt(drag_tangential, p_wcs);
    }
}


void TailHook::applyEffect(RigidBody &rigid, Ptr<DataNode> controls) {
    static const float F_x[] = {-1, 0, 36, 37};
    static const float F_y[] = { 0, 0,  1,  1};
    static const int   nF = sizeof(F_x) / sizeof(float);
    
    if (!partner) {
        Vector p0_wcs = rigid.getState().x + rigid.getState().q.rot(p0);
        Vector p1_wcs = rigid.getState().x + rigid.getState().q.rot(p1);
        Ptr<Collide::Collidable> collidable =
            collision_manager->lineQuery(p0_wcs, p1_wcs, 0, 0, nocollide.lock());
        if (collidable && collidable->getActor()->getTargetInfo()
            && collidable->getActor()->getTargetInfo()->isA(TargetInfo::CARRIER))
        {
            partner = collidable->getRigid();
        }
    }
    if (partner) { // already arrested
        Vector p1_wcs = rigid.getState().x + rigid.getState().q.rot(p1);
        Vector v_rel = rigid.getVelocityAt(p1_wcs) - partner->getVelocityAt(p1_wcs);
        Vector front = rigid.getState().q.rot(Vector(0,0,1));
        float  v_front = v_rel * front;
        
        float force_magnitude = interp(nF, v_front, F_x, F_y);
        rigid.applyForceAt(-max_force * force_magnitude * front, p1_wcs);
    }
}

} // namespace Effectors

