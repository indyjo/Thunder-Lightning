#include "game.h"

void Game::initControls()
{
    Ptr<EventRemapper> r = getEventRemapper();
    r->sig_action_triggered.connect(SigC::slot(*this, &Game::actionTriggered));

    r->map("endgame", SigC::slot(*this, & Game::endGame));
    r->map("debug", SigC::slot(*this, & Game::toggleDebugMode));

    r->setAxis("kbd_throttle",-1.0f);
    r->map("throttle0", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.0f));
    r->map("throttle1", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.11f));
    r->map("throttle2", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.22f));
    r->map("throttle3", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.33f));
    r->map("throttle4", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.44f));
    r->map("throttle5", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.55f));
    r->map("throttle6", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.66f));
    r->map("throttle7", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.77f));
    r->map("throttle8", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.88f));
    r->map("throttle9", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 1.0f));

    r->map("autopilot", SigC::slot(*this, &Game::toggleControlMode));

    r->map("pause", SigC::slot(*this, & Game::togglePauseMode));
    r->map("view0", SigC::bind(
    	SigC::slot(*this, &Game::setView), 0));
    r->map("view1", SigC::bind(
    	SigC::slot(*this, &Game::setView), 1));
    r->map("view2", SigC::bind(
    	SigC::slot(*this, &Game::setView), 2));
    r->map("view3", SigC::bind(
    	SigC::slot(*this, &Game::setView), 3));
    r->map("view4", SigC::bind(
    	SigC::slot(*this, &Game::setView), 4));
    r->map("view5", SigC::bind(
    	SigC::slot(*this, &Game::setView), 5));
    r->map("next-view-subject", SigC::slot(*this, &Game::nextTarget));
    r->map("external-view", SigC::slot(*this, &Game::externalView));

    r->map("faster", SigC::slot(*this, &Game::accelerateSpeed));
    r->map("slower", SigC::slot(*this, &Game::decelerateSpeed));
    
    r->addAxisManipulator(
        AxisManipulator(new LinearAxisTransform(-0.5f, 0.5f), "js_throttle2")
        .input("js_throttle"));

    r->addAxisManipulator(
        AxisManipulator(new LinearAxisTransform(1.0f/5, 0.0f), "mouse_aileron")
        .input("mouse_rel_x"));
    r->addAxisManipulator(
        AxisManipulator(new LinearAxisTransform(1.0f/5, 0.0f), "mouse_elevator")
        .input("mouse_rel_y"));

    r->addAxisManipulator(
        AxisManipulator(new ClampAxisTransform(), "mouse_aileron")
        .input("mouse_aileron"));
    r->addAxisManipulator(
        AxisManipulator(new ClampAxisTransform(), "mouse_elevator")
        .input("mouse_elevator"));
        
    r->map("+left",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_aileron_left", -1.0f));
    r->map("-left",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_aileron_left", 0.0f));
    r->map("+right",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_aileron_right", 1.0f));
    r->map("-right",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_aileron_right", 0.0f));
    r->map("+forward",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_elevator_up", -1.0f));
    r->map("-forward",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_elevator_up", 0.0f));
    r->map("+backward",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_elevator_down", +1.0f));
    r->map("-backward",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_elevator_down", 0.0f));
    r->map("+rudder_left",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_rudder_left", -1.0f));
    r->map("-rudder_left",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_rudder_left", 0.0f));
    r->map("+rudder_right",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_rudder_right", +1.0f));
    r->map("-rudder_right",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_rudder_right", 0.0f));

    r->addAxisManipulator(
        AxisManipulator(new SumAxesTransform(), "aileron")
        .input("mouse_aileron")
        .input("js_aileron")
        .input("kbd_aileron_left")
        .input("kbd_aileron_right"));
    r->addAxisManipulator(
        AxisManipulator(new SumAxesTransform(), "elevator")
        .input("mouse_elevator")
        .input("js_elevator")
        .input("kbd_elevator_up")
        .input("kbd_elevator_down"));
    r->addAxisManipulator(
        AxisManipulator(new SumAxesTransform(), "rudder")
        .input("js_rudder")
        .input("kbd_rudder_left")
        .input("kbd_rudder_right"));
    r->addAxisManipulator(
        AxisManipulator(new SelectAxisByActivityTransform(0.05f, 1.0f), "throttle")
        .input("js_throttle2")
        .input("kbd_throttle"));
        
    r->map("+forward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_throttle", 1.0f));
    r->map("-forward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_throttle", 0.0f));
    r->map("+backward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_brake", 1.0f));
    r->map("-backward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_brake", 0.0f));
    r->map("+left", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_steer_left", -1.0f));
    r->map("-left", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_steer_left", 0.0f));
    r->map("+right", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_steer_right", 1.0f));
    r->map("-right", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_steer_right", 0.0f));
    
    r->map("+forward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_up", 1.0f));
    r->map("-forward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_up", 0.0f));
    r->map("+backward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_down", -1.0f));
    r->map("-backward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_down", 0.0f));
    r->map("+left", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_left", -1.0f));
    r->map("-left", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_left", 0.0f));
    r->map("+right", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_right", 1.0f));
    r->map("-right", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_right", 0.0f));

    r->addAxisManipulator(
        AxisManipulator(new LinearAxisTransform(1.0f/5, 0.0f), "mouse_turret_steer")
        .input("mouse_rel_x"));
    r->addAxisManipulator(
        AxisManipulator(new LinearAxisTransform(-1.0f/5, 0.0f), "mouse_cannon_steer")
        .input("mouse_rel_y"));
    r->addAxisManipulator(
        AxisManipulator(new ClampAxisTransform(), "mouse_turret_steer")
        .input("mouse_turret_steer"));
    r->addAxisManipulator(
        AxisManipulator(new ClampAxisTransform(), "mouse_cannon_steer")
        .input("mouse_cannon_steer"));
    
    r->addAxisManipulator(
    	AxisManipulator(new SumAxesTransform(), "kbd_car_steer")
    	.input("kbd_car_steer_left")
    	.input("kbd_car_steer_right"));
    	
   	r->addAxisManipulator(
   		AxisManipulator(new SumAxesTransform(), "car_steer")
   		.input("kbd_car_steer"));
   	r->addAxisManipulator(
   		AxisManipulator(new SumAxesTransform(), "car_brake")
   		.input("kbd_car_brake"));
   	r->addAxisManipulator(
   		AxisManipulator(new SumAxesTransform(), "car_throttle")
   		.input("kbd_car_throttle"));

   	r->addAxisManipulator(
   		AxisManipulator(new SumAxesTransform(), "tank_turret_steer")
   		.input("mouse_turret_steer"));
   	r->addAxisManipulator(
   		AxisManipulator(new SumAxesTransform(), "tank_cannon_steer")
   		.input("mouse_cannon_steer"));
   		
    // Strafing
    r->addAxisManipulator(
    	AxisManipulator(new SumAxesTransform(), "strafe_horizontal")
    	.input("kbd_strafe_left")
    	.input("kbd_strafe_right"));
    r->addAxisManipulator(
    	AxisManipulator(new SumAxesTransform(), "strafe_vertical")
    	.input("kbd_strafe_up")
    	.input("kbd_strafe_down"));
}

