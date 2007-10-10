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

