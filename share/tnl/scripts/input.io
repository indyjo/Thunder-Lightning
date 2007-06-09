EventRemapper do(
  KeySyms := Object clone
  KeySyms doFile(Config scripts_dir .. "/keysyms.io")
  appendProto(KeySyms)

  press := true
  release := nil
  
  mapKey(SDLK_ESCAPE, press, "endgame")
  mapKey(SDLK_TAB, press,  "map_magnify")
  mapKey(SDLK_TAB, release, "map_demagnify")

  mapKey(SDLK_UP, press, "+forward")
  mapKey(SDLK_UP, release, "-forward")
  mapKey(SDLK_DOWN, press, "+backward")
  mapKey(SDLK_DOWN, release, "-backward")
  mapKey(SDLK_LEFT, press, "+left")
  mapKey(SDLK_LEFT, release, "-left")
  mapKey(SDLK_RIGHT, press, "+right")
  mapKey(SDLK_RIGHT, release, "-right")
  
  mapKey(SDLK_COMMA, press, "+rudder_left")
  mapKey(SDLK_COMMA, release, "-rudder_left")
  mapKey(SDLK_PERIOD, press, "+rudder_right")
  mapKey(SDLK_PERIOD, release, "-rudder_right")

  mapKey(SDLK_1, press, "throttle0")
  mapKey(SDLK_2, press, "throttle1")
  mapKey(SDLK_3, press, "throttle2")
  mapKey(SDLK_4, press, "throttle3")
  mapKey(SDLK_5, press, "throttle4")
  mapKey(SDLK_6, press, "throttle5")
  mapKey(SDLK_7, press, "throttle6")
  mapKey(SDLK_8, press, "throttle7")
  mapKey(SDLK_9, press, "throttle8")
  mapKey(SDLK_0, press, "throttle9")
  mapKey(SDLK_a, press, "autopilot")
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
  mapKey(SDLK_BACKSPACE, press, "cycle-primary")
  mapKey(SDLK_RETURN,  press, "cycle-secondary")
  mapKey(SDLK_LCTRL, press,  "+primary")
  mapKey(SDLK_LCTRL, release, "-primary")
  mapKey(SDLK_SPACE, press,  "+secondary")
  mapKey(SDLK_SPACE, release, "-secondary")
  mapKey(SDLK_RCTRL, press,  "+tertiary")
  mapKey(SDLK_RCTRL, release, "-tertiary")
  
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
  mapKey(SDLK_b, press, "+brake")
  mapKey(SDLK_b, release, "-brake")
  
  mapKey(SDLK_i, press, "toggle-introduction")
  
  if (Config Controls_enable_mouse != "false",
    mapMouseButton(1, press,  "+primary")
    mapMouseButton(1, release, "-primary")
    mapMouseButton(3, press,  "+secondary")
    mapMouseButton(3, release, "-secondary")
    mapMouseButton(2, press,  "+tertiary")
    mapMouseButton(2, release, "-tertiary")
    mapMouseButton(4, press,  "cycle-secondary")
    mapMouseButton(5, press,  "cycle-secondary")
    
    mapRelativeMouseAxes("mouse_rel_x", "mouse_rel_y")
    mapAbsoluteMouseAxes("mouse_abs_x", "mouse_abs_y")
  )
  
  if (Config Controls_enable_joystick != "false",
    mapJoystickButton(0, 0, press,  "+primary")
    mapJoystickButton(0, 0, release, "-primary")
    mapJoystickButton(0, 1, press,  "+secondary")
    mapJoystickButton(0, 1, release, "-secondary")
    mapJoystickButton(0, 2, press,  "gunsight_target")
    mapJoystickButton(0, 3, press,  "cycle-secondary")
  
    mapJoystickAxis(0,0,"js_aileron")
    mapJoystickAxis(0,1,"js_elevator")
    mapJoystickAxis(0,3,"js_rudder")
    mapJoystickAxis(0,2,"js_throttle")
  )
)
