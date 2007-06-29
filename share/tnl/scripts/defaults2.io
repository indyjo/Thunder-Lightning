// These configuration values are set _after_ config.io, either because they
// depend on a value set there or because they should override some value.

Config do(
  // Only grab the mouse if the game is to run fullscreen
  Game_grab_mouse := Game_fullscreen
  
  // Camera configuration
  Camera_focus                     := "1.5"
  Camera_aspect                    := ((Game_xres asNumber / Game_yres asNumber) asString)
)


