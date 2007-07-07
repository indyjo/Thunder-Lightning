Config do(
  // Whether or not the game should run fullscreen ("true") or windowed ("false")
  Game_fullscreen := "true"
  
  // Use automatically detected resolution (only if Game_fullscreen is "true")
  Game_auto_resolution := "true"
  
  // Screen resolution (if Game_autodetect_resolution is "false")
  Game_xres       := "800"
  Game_yres       := "600"
  
  // Minimum bit numbers for color and depth channels
  Game_red_bits     := "5"
  Game_green_bits   := "5"
  Game_blue_bits    := "5"
  Game_zbuffer_bits := "1"
  
  // Full screen antialiasing.
  Game_fsaa_enabled := "false"
  Game_fsaa_samples := "2"
  
  // GLSL shader support
  Game_use_shaders := "true"
  
  // Mirror created by ocean surface.
  // Higher values result in a nicer water rendering, lower values improve
  // performance. Only power-of-two values are allowed. Maximum is Game_yres
  Water_mirror_texture_size := "512"
  
  // Use GLSL shaders for water. Only effective if Game_use_shaders is false
  Water_use_shaders := "true"
  
  // Controls config
  Controls_enable_joystick := "true"
  Controls_enable_mouse    := "true"
  // Joystick sensitivity. Higher values are actually _less_ sensitive.
  // A value of 0 corresponds with a linear response. 1 is quadratic,
  // 2 is cubic etc. Floating-point values are allowed.
  Controls_joystick_sensitivity := "2"
) // Config do

