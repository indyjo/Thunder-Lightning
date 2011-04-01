EventRemapper do(
  KeySyms := Object clone
  KeySyms doFile(Config scripts_dir .. "/keysyms.io")
  appendProto(KeySyms)

  press := true
  release := nil
  
  mapKey(SDLK_ESCAPE, press, "endgame")
  mapKey(SDLK_TAB, press,  "switch-mfd")

  mapKey(SDLK_UP, press, "+forward")
  mapKey(SDLK_UP, release, "-forward")
  mapKey(SDLK_DOWN, press, "+backward")
  mapKey(SDLK_DOWN, release, "-backward")
  mapKey(SDLK_LEFT, press, "+left")
  mapKey(SDLK_LEFT, release, "-left")
  mapKey(SDLK_RIGHT, press, "+right")
  mapKey(SDLK_RIGHT, release, "-right")
  
  mapKey(SDLK_SEMICOLON, press, "+rudder_left")
  mapKey(SDLK_SEMICOLON, release, "-rudder_left")
  mapKey(SDLK_COLON, press, "+rudder_right")
  mapKey(SDLK_COLON, release, "-rudder_right")

  mapKey(SDLK_AMPERSAND,  press, "throttle0")
  mapKey(SDLK_WORLD_73,   press, "throttle1")
  mapKey(SDLK_QUOTEDBL,   press, "throttle2")
  mapKey(SDLK_QUOTE,      press, "throttle3")
  mapKey(SDLK_LEFTPAREN,  press, "throttle4")
  mapKey(SDLK_MINUS,      press, "throttle5")
  mapKey(SDLK_WORLD_72,   press, "throttle6")
  mapKey(SDLK_UNDERSCORE, press, "throttle7")
  mapKey(SDLK_WORLD_71,   press, "throttle8")
  mapKey(SDLK_WORLD_64,   press, "throttle9")
  mapKey(SDLK_a, press, "autopilot")
  
  mapKey(SDLK_ASTERISK, press, "increase-throttle")
  mapKey(SDLK_KP_PLUS,  press, "increase-throttle")

  mapKey(SDLK_WORLD_89, press, "decrease-throttle")
  mapKey(SDLK_KP_MINUS, press, "decrease-throttle")
  
  mapKey(SDLK_p, press, "pause")
  mapKey(SDLK_F11, press, "toggle-console")

  mapKey(SDLK_F1, press, "view0")
  mapKey(SDLK_F2, press, "view1")
  mapKey(SDLK_F3, press, "view2")
  mapKey(SDLK_F4, press, "view3")
  mapKey(SDLK_F5, press, "view4")
  mapKey(SDLK_F6, press, "view5")

  mapKey(SDLK_F7, press, "zoom-out")
  mapKey(SDLK_F8, press, "zoom-in")

  mapKey(SDLK_n, press, "next-view-subject")
  mapKey(SDLK_v, press, "external-view")
  mapKey(SDLK_RETURN,  press, "cycle-primary")
  mapKey(SDLK_LCTRL, press,  "+primary")
  mapKey(SDLK_LCTRL, release, "-primary")
  
  mapKey(SDLK_F12, press, "debug")

  mapKey(SDLK_r, press, "previous-target")
  mapKey(SDLK_t, press, "next-target")
  mapKey(SDLK_h, press, "next-hostile-target")
  mapKey(SDLK_f, press, "next-friendly-target")
  mapKey(SDLK_g, press, "gunsight-target")
  mapKey(SDLK_z, press, "nearest-target")
  mapKey(SDLK_y, press, "nearest-target")
  
  mapKey(SDLK_F9, press, "slower")
  mapKey(SDLK_F10, press, "faster")
  
  mapKey(SDLK_l, press, "landing-gear")
  mapKey(SDLK_k, press, "landing-hook")
  mapKey(SDLK_b, press, "+brake")
  mapKey(SDLK_b, release, "-brake")
  
  mapKey(SDLK_i, press, "toggle-introduction")
  
  if (Config Controls_enable_mouse != "false",
    mapMouseButton(1, press,  "+primary")
    mapMouseButton(1, release, "-primary")
    mapMouseButton(2, press,  "gunsight-target")
    mapMouseButton(3, press,  "cycle-primary")
    
    mapMouseButton(1, press,   "+observer-dolly")
    mapMouseButton(1, release, "-observer-dolly")
    mapMouseButton(3, press,   "+observer-pan")
    mapMouseButton(3, release, "-observer-pan")
    mapMouseButton(2, press,   "observer-stop")
    
    mapRelativeMouseAxes("mouse_rel_x", "mouse_rel_y")
    mapAbsoluteMouseAxes("mouse_abs_x", "mouse_abs_y")
  )
  
  if (Config Controls_enable_joystick != "false",
    mapJoystickButton(0, 0, press,  "+primary")
    mapJoystickButton(0, 0, release, "-primary")
    mapJoystickButton(0, 1, press,  "cycle-primary")
    mapJoystickButton(0, 2, press,  "gunsight-target")
    mapJoystickButton(0, 3, press,  "next-friendly-target")
    mapJoystickButton(0, 4, press,  "next-hostile-target")
    mapJoystickButton(0, 5, press,  "+brake")
    mapJoystickButton(0, 5, release,"-brake")
    mapJoystickButton(0, 7, press,  "landing-gear")
    mapJoystickButton(0, 8, press,  "landing-hook")
  
    mapJoystickAxis(0,0,"js_aileron")
    mapJoystickAxis(0,1,"js_elevator")
    mapJoystickAxis(0,3,"js_rudder")
    mapJoystickAxis(0,2,"js_throttle")
  )
)
