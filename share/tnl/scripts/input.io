EventRemapper do(
  KeySyms := Object clone
  KeySyms doFile(Config scripts_dir .. "/keysyms.io")
  appendProto(KeySyms)
  
  keySymOf := method(code,
    KeySyms slotNames foreach(slotname,
      if (KeySyms getSlot(slotname) == code,
        return slotname
      )
    )
    
    code asString
  )
  
  registerAction("mainmenu", "Main menu", "Enter main menu while playing.")
  registerAction("switch-mfd", "Switch MFD mode", "Cycle between the available modes of the multi function display (MFD).")
  registerAction("+forward", "Forward/down", "Accelerate ground-based vehicle. For aircraft, push elevator (nose down).")
  registerAction("+backward", "Backward/brake/up", "Brake and reverse vehicle. For aircraft, pull elevator (nose up).")
  registerAction("+left", "Left", "Steer vehicle left. For aircraft, aileron left (roll counter-clockwise).")
  registerAction("+right", "Right", "Steer vehicle right. For aircraft, aileron right (roll clockwise).")
  
  registerAction("+rudder_left", "Rudder left (aircraft)", "Rudder sharp left in aircraft. Causes nose to yaw left. Also steers front wheel.")
  registerAction("+rudder_right", "Rudder right (aircraft)", "Rudder sharp right in aircraft. Causes nose to yaw right. Also steers front wheel.")

  registerAction("throttle0", "Throttle 0%", "Sets throttle to 0%.")
  registerAction("throttle1", "Throttle 11%", "Sets throttle to 11%.")
  registerAction("throttle2", "Throttle 22%", "Sets throttle to 22%.")
  registerAction("throttle3", "Throttle 33%", "Sets throttle to 33%.")
  registerAction("throttle4", "Throttle 44%", "Sets throttle to 44%.")
  registerAction("throttle5", "Throttle 55%", "Sets throttle to 55%.")
  registerAction("throttle6", "Throttle 66%", "Sets throttle to 66%.")
  registerAction("throttle7", "Throttle 77%", "Sets throttle to 77%.")
  registerAction("throttle8", "Throttle 88%", "Sets throttle to 88%.")
  registerAction("throttle9", "Throttle 100%", "Sets throttle to 100%.")
  registerAction("increase-throttle", "Throttle: increase by 5%", "")
  registerAction("decrease-throttle", "Throttle: decrease by 5%", "")

  registerAction("autopilot", "Automatic/manual control", "Toggle the current unit's control mode between automatic and manual.")
  
  registerAction("pause", "Pause", "Toggle game state between paused and unpaused.")
  registerAction("toggle-console", "Console", "Switch console on/off.")

  registerAction("view0", "View 1", "Switch to view 1 (main view) of current unit.")
  registerAction("view1", "View 2", "Switch to view 2 (if available) of current unit.")
  registerAction("view2", "View 3", "Switch to view 3 (if available) of current unit.")
  registerAction("view3", "View 4", "Switch to view 4 (if available) of current unit.")
  registerAction("view4", "View 5", "Switch to view 5 (if available) of current unit.")
  registerAction("view5", "View 6", "Switch to view 6 (if available) of current unit.")

  registerAction("next-view-subject", "Next unit", "Switch to next unit from same faction. Previous unit is set to automatic control.")
  registerAction("external-view", "Observer view", "Switch to/from external observer view.")
  registerAction("cycle-primary", "Weapon", "Cycle between weapons.")
  registerAction("+primary", "Fire", "Fire currently selected weapon.")
  
  registerAction("debug", "Debug mode", "Toggle debug mode on/off.")

  registerAction("previous-target", "Target previous", "Target previous radar contact.")
  registerAction("next-target", "Target next", "Target next radar contact.")
  registerAction("next-hostile-target", "Target next hostile", "Target next hostile radar contact.")
  registerAction("next-friendly-target", "Target next friendly", "Target next friendly radar contact.")
  registerAction("gunsight-target", "Target object in gunsight", "Target object closest to center of gunsight.")
  registerAction("nearest-target", "Target nearest", "Target nearest radar contact.")
  
  registerAction("slower", "Time warp: slower", "Decelerate time, causing slow-motion.")
  registerAction("faster", "Time warp: faster", "Accelerate time, causing fast-forward.")
  
  registerAction("landing-gear", "Landing gear", "Lower/raise landing gear.")
  registerAction("landing-hook", "Landing hook", "Lower/raise landing hook for carrier landings.")
  registerAction("+brake", "Air brake", "Deploy air brake while held.")
  
  registerAction("+observer-dolly", "Observer: dolly", "While pressed, mouse movements affect observer forward/backward motion.")
  registerAction("+observer-pan", "Observer: pan", "While pressed, mouse movements cause observer left/right and up/down motion.")
  registerAction("observer-stop", "Observer: stop", "Stop the observer immediately.")
  
  registerAxis("throttle", "Throttle", "Throttle for aircraft and the carrier.")
  registerAxis("aileron", "Aileron", "Aircraft aileron. Controls the aircraft's roll speed.")
  registerAxis("elevator", "Elevator", "Aircraft elevator. Controls the aircraft's pitch speed.")
  registerAxis("rudder", "Rudder", "Aircraft rudder. Controls the aircraft's yaw speed.")
  registerAxis("brake", "Brakes", "Aircraft brakes. Air brakes control speed of descent. Wheel brakes decelerate the aircraft on the ground.")
  registerAxis("car_throttle", "Throttle (vehicle)", "Throttle for land-based vehicles.")
  registerAxis("car_brake", "Brake (vehicle)", "Brake for land-based vehicles.")
  registerAxis("car_steer", "Steering wheel (vehicle)", "Steering for land-based vehicles.")
  registerAxis("turret_steer", "Azimuth (turret)", "Horizontal turret control on tanks and carrier.")
  registerAxis("cannon_steer", "Elevation (turret)", "Vertical turret control on tanks and carrier.")

  mapKey(SDLK_ESCAPE, "mainmenu")
  mapKey(SDLK_TAB,  "switch-mfd")

  mapKey(SDLK_UP, "+forward")
  mapKey(SDLK_DOWN, "+backward")
  mapKey(SDLK_LEFT, "+left")
  mapKey(SDLK_RIGHT, "+right")
  mapKey(SDLK_COMMA, "+rudder_left")
  mapKey(SDLK_PERIOD, "+rudder_right")

  mapKey(SDLK_1, "throttle0")
  mapKey(SDLK_2, "throttle1")
  mapKey(SDLK_3, "throttle2")
  mapKey(SDLK_4, "throttle3")
  mapKey(SDLK_5, "throttle4")
  mapKey(SDLK_6, "throttle5")
  mapKey(SDLK_7, "throttle6")
  mapKey(SDLK_8, "throttle7")
  mapKey(SDLK_9, "throttle8")
  mapKey(SDLK_0, "throttle9")
  mapKey(SDLK_a, "autopilot")
  mapKey(SDLK_PLUS,  "increase-throttle")
  mapKey(SDLK_MINUS, "decrease-throttle")
  
  mapKey(SDLK_p, "pause")
  mapKey(SDLK_F11, "toggle-console")

  mapKey(SDLK_F1, "view0")
  mapKey(SDLK_F2, "view1")
  mapKey(SDLK_F3, "view2")
  mapKey(SDLK_F4, "view3")
  mapKey(SDLK_F5, "view4")
  mapKey(SDLK_F6, "view5")

  mapKey(SDLK_F7, "zoom-out")
  mapKey(SDLK_F8, "zoom-in")

  mapKey(SDLK_n, "next-view-subject")
  mapKey(SDLK_v, "external-view")
  mapKey(SDLK_RETURN,  "cycle-primary")
  mapKey(SDLK_LCTRL,  "+primary")
  
  mapKey(SDLK_F12, "debug")

  mapKey(SDLK_r, "previous-target")
  mapKey(SDLK_t, "next-target")
  mapKey(SDLK_h, "next-hostile-target")
  mapKey(SDLK_f, "next-friendly-target")
  mapKey(SDLK_g, "gunsight-target")
  mapKey(SDLK_z, "nearest-target")
  mapKey(SDLK_y, "nearest-target")
  
  mapKey(SDLK_F9, "slower")
  mapKey(SDLK_F10, "faster")
  
  mapKey(SDLK_l, "landing-gear")
  mapKey(SDLK_k, "landing-hook")
  mapKey(SDLK_b, "+brake")
  
  mapKey(SDLK_i, "toggle-introduction")
  
  if (Config Controls_enable_mouse != "false",
    mapMouseButton(1,  "+primary")
    mapMouseButton(2,  "gunsight-target")
    mapMouseButton(3,  "cycle-primary")
    
    mapMouseButton(1,   "+observer-dolly")
    mapMouseButton(3,   "+observer-pan")
    mapMouseButton(2,   "observer-stop")
    
    mapRelativeMouseAxes("mouse_rel_x", "mouse_rel_y")
    mapAbsoluteMouseAxes("mouse_abs_x", "mouse_abs_y")
  )
  
  if (Config Controls_enable_joystick != "false",
    mapJoystickButton(0, 0,  "+primary")
    mapJoystickButton(0, 1,  "cycle-primary")
    mapJoystickButton(0, 2,  "gunsight-target")
    mapJoystickButton(0, 3,  "next-friendly-target")
    mapJoystickButton(0, 4,  "next-hostile-target")
    mapJoystickButton(0, 5,  "+brake")
    mapJoystickButton(0, 7,  "landing-gear")
    mapJoystickButton(0, 8,  "landing-hook")
  
    mapJoystickAxis(0,0,"js_aileron")
    mapJoystickAxis(0,1,"js_elevator")
    mapJoystickAxis(0,3,"js_rudder")
    mapJoystickAxis(0,2,"js_throttle")
  )
)

