#include <interfaces/IConfig.h>
#include "game.h"

namespace {
    void incAxis(EventRemapper *r, const char *axis) {
        r->setAxis(axis, std::max(0.0f, std::min(1.0f, r->getAxis(axis) + 0.05f)));
    }
    void decAxis(EventRemapper *r, const char *axis) {
        r->setAxis(axis, std::max(0.0f, std::min(1.0f, r->getAxis(axis) - 0.05f)));
    }
}

void Game::initControls()
{
    Ptr<EventRemapper> r = getEventRemapper();
    
    r->sig_action_triggered.connect(SigC::slot(*this, &Game::actionTriggered));

    event_sheet->map("mainmenu", SigC::slot(*this, & Game::mainMenu));
    event_sheet->map("debug", SigC::slot(*this, & Game::toggleDebugMode));

    r->setAxis("kbd_throttle",-1.0f);
    event_sheet->map("throttle0", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.0f));
    event_sheet->map("throttle1", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.11f));
    event_sheet->map("throttle2", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.22f));
    event_sheet->map("throttle3", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.33f));
    event_sheet->map("throttle4", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.44f));
    event_sheet->map("throttle5", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.55f));
    event_sheet->map("throttle6", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.66f));
    event_sheet->map("throttle7", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.77f));
    event_sheet->map("throttle8", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 0.88f));
    event_sheet->map("throttle9", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_throttle", 1.0f));
    event_sheet->map("increase-throttle", SigC::bind(
            SigC::slot(&incAxis), ptr(r), "kbd_throttle"));
    event_sheet->map("decrease-throttle", SigC::bind(
            SigC::slot(&decAxis), ptr(r), "kbd_throttle"));

    event_sheet->map("autopilot", SigC::slot(*this, &Game::toggleControlMode));

    event_sheet->map("pause", SigC::slot(*this, & Game::togglePauseMode));
    event_sheet->map("view0", SigC::bind(
    	SigC::slot(*this, &Game::setView), 0));
    event_sheet->map("view1", SigC::bind(
    	SigC::slot(*this, &Game::setView), 1));
    event_sheet->map("view2", SigC::bind(
    	SigC::slot(*this, &Game::setView), 2));
    event_sheet->map("view3", SigC::bind(
    	SigC::slot(*this, &Game::setView), 3));
    event_sheet->map("view4", SigC::bind(
    	SigC::slot(*this, &Game::setView), 4));
    event_sheet->map("view5", SigC::bind(
    	SigC::slot(*this, &Game::setView), 5));
    event_sheet->map("next-view-subject", SigC::slot(*this, &Game::nextTarget));
    event_sheet->map("external-view", SigC::slot(*this, &Game::externalView));

    event_sheet->map("faster", SigC::slot(*this, &Game::accelerateSpeed));
    event_sheet->map("slower", SigC::slot(*this, &Game::decelerateSpeed));
    
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

    float sensitivity = config->queryFloat("Controls_joystick_sensitivity", 1.0);
    r->addAxisManipulator(
        AxisManipulator(new SensitivityAxisTransform(sensitivity), "js_aileron_scaled")
        .input("js_aileron"));
    r->addAxisManipulator(
        AxisManipulator(new SensitivityAxisTransform(sensitivity), "js_elevator_scaled")
        .input("js_elevator"));
    r->addAxisManipulator(
        AxisManipulator(new SensitivityAxisTransform(sensitivity), "js_rudder_scaled")
        .input("js_rudder"));
        
        
    event_sheet->map("+left",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_aileron_left", -1.0f));
    event_sheet->map("-left",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_aileron_left", 0.0f));
    event_sheet->map("+right",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_aileron_right", 1.0f));
    event_sheet->map("-right",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_aileron_right", 0.0f));
    event_sheet->map("+forward",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_elevator_up", -1.0f));
    event_sheet->map("-forward",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_elevator_up", 0.0f));
    event_sheet->map("+backward",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_elevator_down", +1.0f));
    event_sheet->map("-backward",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_elevator_down", 0.0f));
    event_sheet->map("+rudder_left",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_rudder_left", -1.0f));
    event_sheet->map("-rudder_left",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_rudder_left", 0.0f));
    event_sheet->map("+rudder_right",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_rudder_right", +1.0f));
    event_sheet->map("-rudder_right",  SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_rudder_right", 0.0f));
    event_sheet->map("+brake", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_brake", 1.0));
    event_sheet->map("-brake", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_brake", 0.0));

    r->addAxisManipulator(
        AxisManipulator(new SumAxesTransform(), "aileron")
        .input("mouse_aileron")
        .input("js_aileron_scaled")
        .input("kbd_aileron_left")
        .input("kbd_aileron_right"));
    r->addAxisManipulator(
        AxisManipulator(new SumAxesTransform(), "elevator")
        .input("mouse_elevator")
        .input("js_elevator_scaled")
        .input("kbd_elevator_up")
        .input("kbd_elevator_down"));
    r->addAxisManipulator(
        AxisManipulator(new SumAxesTransform(), "rudder")
        .input("js_rudder_scaled")
        .input("kbd_rudder_left")
        .input("kbd_rudder_right"));
    r->addAxisManipulator(
        AxisManipulator(new SelectAxisByActivityTransform(0.025f, 0.0f), "throttle")
        .input("js_throttle2")
        .input("kbd_throttle"));
    r->addAxisManipulator(
        AxisManipulator(new SumAxesTransform, "brake")
        .input("kbd_brake"));
        
    event_sheet->map("+forward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_throttle", 1.0f));
    event_sheet->map("-forward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_throttle", 0.0f));
    event_sheet->map("+backward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_brake", 1.0f));
    event_sheet->map("-backward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_brake", 0.0f));
    event_sheet->map("+left", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_steer_left", -1.0f));
    event_sheet->map("-left", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_steer_left", 0.0f));
    event_sheet->map("+right", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_steer_right", 1.0f));
    event_sheet->map("-right", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_car_steer_right", 0.0f));
    
    event_sheet->map("+forward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_up", 1.0f));
    event_sheet->map("-forward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_up", 0.0f));
    event_sheet->map("+backward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_down", -1.0f));
    event_sheet->map("-backward", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_down", 0.0f));
    event_sheet->map("+left", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_left", -1.0f));
    event_sheet->map("-left", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_left", 0.0f));
    event_sheet->map("+right", SigC::bind(
            SigC::slot(*r, &EventRemapper::setAxis), "kbd_strafe_right", 1.0f));
    event_sheet->map("-right", SigC::bind(
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

