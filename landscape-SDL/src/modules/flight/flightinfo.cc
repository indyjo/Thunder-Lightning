#include <cmath>
#include "flightinfo.h"
#include <interfaces/IMovementProvider.h>
#include <interfaces/ITerrain.h>

#define PI 3.14159265358979323846

using namespace std;

void FlightInfo::update( double delta_t, IMovementProvider & mp, ITerrain & ter) {
    Vector v = mp.getMovementVector();
    acceleration = (v - velocity) / delta_t;
    velocity = v;
    
    p = mp.getLocation();
    Vector up, right, front;
    mp.getOrientation(&up, &right, &front);
    
    // Calculating angular velocity (omega)
    // in order to avoid numerical instabilities we first find out
    // whether the front or the up vector changes faster and then take
    // this vector to calculate the angular velocity
    // We also have to take into account that there might in fact not
    // be any angular velocity at all, in which case we just set omega
    // to 0,0,0
    {
	    Vector old_vec, new_vec;
	    if ((this->up - up).lengthSquare() > (this->front - front).lengthSquare()) {
	    	old_vec = this->up;
	    	new_vec = up;
	    } else {
	    	old_vec = this->front;
	    	new_vec = front;
	    }
	    float angle = acos(old_vec*new_vec);
	    if ((old_vec-new_vec).lengthSquare() < 1e-4) { omega = Vector(0,0,0); }
	    else {
	    	omega = (old_vec % new_vec).normalize() * angle;
	    }
    }
    this->up = up;
    this->right = right;
    this->front = front;
    
    Vector collide_point;
    collision_warning = ter.lineCollides(
            p + Vector(0, -30, 0),
            p + Vector(0, -30, 0) + 4.0 * v,
            &collide_point);

    float new_altitude = p[1];
    d_altitude = (new_altitude - altitude) / delta_t;
    altitude = new_altitude;
    
    float new_height = altitude - ter.getHeightAt(p[0], p[2]);
    d_height = (new_height - height) / delta_t;
    height = new_height;
    
    float new_course = atan2( front[0], front[2] );
    if (new_course < 0) new_course += 2*PI;
    d_course = new_course - course;
    course = new_course;
    if (d_course >  PI) d_course -= 2*PI;
    if (d_course < -PI) d_course += 2*PI;
    d_course /= delta_t;

    right.normalize();
    front.normalize();

    float old_speed = speed;
    speed = v * front;
    accel = (speed - old_speed) / delta_t;
    if (accel < -10.0) accel = -10.0;
    if (accel > +10.0) accel = +10.0;

    Vector up2( right % Vector(front[0], 0, front[2]) );
    up2.normalize();
    
    Vector right2 = (Vector(0,1,0) % front).normalize();

    float new_roll;
    new_roll = acos( min(1.0f, max(-1.0f, right * right2)) );
    if ( (right2 % right) * front > 0.0) new_roll = -new_roll;
    d_roll = (new_roll - roll) / delta_t;
    roll = new_roll;
    
    float new_pitch = asin(front * Vector(0,1,0));
    d_pitch = (new_pitch - pitch) / delta_t;
    pitch = new_pitch;
    
    {
        Vector up(front % right);
        up.normalize();
        v.normalize();
        aoa = -asin( v*up );
        //aoa = acos( v.normalize()*front );
    }
    
    //ls_message("ground height at pos %4.2f %4.2f is %4.2f\n", loc[0], loc[2],
    //        ter.getHeightAt(loc[0], loc[2]));
}

void FlightInfo::dump() {
    ls_message("Flight info dump follows >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    ls_message("height = %4.2f course = %4.2f speed = %4.2f\n", height, course, speed);
    ls_message("roll = %3.2f pitch=%3.2f\n", roll, pitch);
    ls_message("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
};
