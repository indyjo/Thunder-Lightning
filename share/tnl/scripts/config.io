Config do(
  old_setSlot := getSlot("setSlot")
  
  // This definition makes Config call query(key) for every slot not found in Config
  forward := method(
    query(call message name)
  )
  
  // The following line change the behavior of "setSlot" and "setSlotWithType"
  // to call the set(key, value) method.
  // This is very tricky, don't modify if you don't have to!
  configSetSlot := method(key, value,
    "In setSlot!" println
    if (hasSlot(key), old_setSlot(key,value), set(key,value))
    write("Config: " .. key .. "=" .. value .. "\n")
    value
  )
  setSlotWithType := getSlot("configSetSlot")
  setSlot := getSlot("configSetSlot")
  
  // -------------------------------------------------------------------------
  // ------- After this point, all assignments are configuration data! -------
  // -------------------------------------------------------------------------
  
  terrain_dir := data_dir .. "/lod-terrains/compare"
  texture_dir := data_dir .. "/textures"
  model_dir   := data_dir .. "/models"
  fonts_dir   := data_dir .. "/fonts"
  sounds_dir  := data_dir .. "/sounds"
  scripts_dir := data_dir .. "/scripts"
  shaders_dir := data_dir .. "/shaders"
  
  
  // Try to restore graphics and keyboard after a program crash
  Game_enable_SDL_parachute := "true"
  
  // Whether or not the game should run fullscreen ("true") or windowed ("false")
  Game_fullscreen := "true"
  
  // Only grab the mouse if the game is to run fullscreen
  Game_grab_mouse := Game_fullscreen
  
  // Only autodetect resolution if the game is to run fullscreen
  Game_auto_resolution := Game_fullscreen
  
  // Resolution if Game_autodetect_resolution is set to "false"
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
  
  Game_loading_screen := texture_dir .. "/loading-screen.spr"
  
  // Mirror created by ocean surface
  Game_mirror_texture_size := "512"
  
  // Controls config
  Controls_enable_joystick := "true"
  Controls_enable_mouse    := "true"

  // Environment config
  Environment_clip_min      := "0.2"
  Environment_clip_max      := "20000"
  Environment_fog_r         := (202/255) asString
  Environment_fog_g         := (164/255) asString
  Environment_fog_b         := (145/255) asString
  Environment_ground_fog_min    := "0.0"
  Environment_ground_fog_max    := "400.0"
  Environment_ground_fog_range    := "3500.0"

  // Sound config
  SoundMan_sound_dir        := sounds_dir
  SoundMan_doppler_velocity := "5000"
  SoundMan_doppler_factor   := "1"
  SoundMan_channels         := "64"
  SoundMan_minimum_gain     := "0.001"
  SoundMan_hysteresis       := "0.01"

  // Level of Detail Terrain Manager configuration
  LoDQuadManager_terrain_prefix    := terrain_dir .. "/terrain"
  LoDQuadManager_texture_prefix    := terrain_dir .. "/texture"
  LoDQuadManager_texmap_prefix     := terrain_dir .. "/texmap"
  LoDQuadManager_lightmap_prefix   := terrain_dir .. "/lightmap"
  LoDQuadManager_quads_w           := "1"
  LoDQuadManager_quads_h           := "1"
  LoDQuadManager_detail_tex_name   := texture_dir .. "/detail-texture.spr"
  LoDQuadManager_tile_dir          := texture_dir .. "/terrain-tiles"
  LoDQuadManager_texture_list      := terrain_dir .. "/textures"

  // Map configuration
  Map_texture_file                 := terrain_dir .. "/map.spr"

  // Camera configuration
  Camera_focus                     := "1.5"
  Camera_aspect                    := ((Game_xres asNumber / Game_yres asNumber) asString)

  // Sky (clouds layer) configuration
  Sky_upside_texture_path          := texture_dir .. "/sky-layer1-above.spr"
  Sky_downside_texture_path        := texture_dir .. "/sky-layer1-below.spr"

  // SkyBox configuration
  SkyBox_texture_path              := texture_dir .. "/skybox/pastel"

  // Gunsight configuration
  Gunsight_texture_file            := texture_dir .. "/gunsight.spr"

  // SmokeTrail configuration
  SmokeTrail_smoke_tex             := texture_dir .. "/missile_smoke.spr"
  SmokeTrail_puffy_tex             := texture_dir .. "/puffy_smoke.spr"

  // SmokeColumn configuration
  SmokeColumn_puffy_tex            := texture_dir .. "/puffy_smoke.spr"

  // Explosion configuration
  Explosion_frames                 := "16"
  Explosion_seconds_per_frame      := "0.08"
  Explosion_filename_prefix        := texture_dir .. "/explosion/explosion"
  Explosion_filename_postfix       := ".png"
  Explosion_size                   := "20.0"
  Explosion_sound                  := "explosion-01.wav"

  // Drone configuration
  Drone_model_path                 := model_dir .. "/lightning"
  Drone_skeleton                   := Drone_model_path .. "/lightning.spec"
  Drone_inside_model_file          := Drone_model_path .. "/lightning-with-interior.obj"
  Drone_outside_model_file         := Drone_model_path .. "/lightning.obj"
  Drone_model_bounds               := Drone_model_path .. "/lightning.bounds"
  Drone_target_radius              := "3.5"
  Drone_pilot_pos                  := "Vector3( 0 0.8 1.9)"
  Drone_cannon_sound               := "cannon-shoot-1.wav"
  Drone_engine_sound               := "aircraft-engine-1.wav"
  Drone_engine_gain                := "0.1"
  Drone_engine_gain_lowest         := "0.02"
  Drone_engine_gain_highest        := "0.1"
  Drone_engine_pitch_lowest        := "0.5"
  Drone_engine_pitch_highest       := "2.0"
  Drone_wheel_model_file           := model_dir .. "/misc/tire.obj"
  
  Drone_NoseWheel_range            := "1.2"
  Drone_NoseWheel_force            := "100000"
  Drone_NoseWheel_damping          := "15000"
  Drone_NoseWheel_C_tan_min        := "50"
  Drone_NoseWheel_C_tan_max        := "50"
  Drone_NoseWheel_C_norm           := "10000"

  Drone_LeftWheel_range            := "0.8"
  Drone_LeftWheel_force            := "50000"
  Drone_LeftWheel_damping          := "15000"
  Drone_LeftWheel_C_tan_min        := "50"
  Drone_LeftWheel_C_tan_max        := "1000"
  Drone_LeftWheel_C_norm           := "1000"

  Drone_RightWheel_range           := Drone_LeftWheel_range
  Drone_RightWheel_force           := Drone_LeftWheel_force
  Drone_RightWheel_damping         := Drone_LeftWheel_damping
  Drone_RightWheel_C_tan_min       := Drone_LeftWheel_C_tan_min
  Drone_RightWheel_C_tan_max       := Drone_LeftWheel_C_tan_max
  Drone_RightWheel_C_norm          := Drone_LeftWheel_C_norm

  Drone_max_tailhook_force         := "40000"

  // Tank configuration
  Tank_model_path                  := model_dir .. "/newtank"
  Tank_skeleton                    := Tank_model_path .. "/Tank.spec"
  Tank_model_bounds                := Tank_model_path .. "/Tank.bounds"
  Tank_cannon_sound                := "cannon-shoot-1.wav"
  Tank_engine_sound_low            := "car-engine-1-loop.wav"
  Tank_engine_sound_high           := "car-engine-2-loop.wav"
  Tank_cannon_factor               := "10"
  Tank_vulcan_factor               := "0.8"

  // FontMan configuration
  FontMan_dir                      := fonts_dir

  // Io Scripting configuration
  Io_init_script                   := scripts_dir .. "/init.io"
  Io_init_script_2                 := scripts_dir .. "/init2.io"
  
  DumbMissile_model                := model_dir .. "/missiles/dumbmissile.obj"
  SmartMissile_model               := model_dir .. "/missiles/smartmissile.obj"
  Missile_engine_sound             := "rocket-engine-1.wav"

  Missile_Kp                       := "1"
  Missile_Ki                       := "0"
  Missile_Kd                       := "5"
  
  // Weapons configuration
  Cannon_sound                     := "cannon-shoot-1.wav"
  Cannon_muzzle_velocity           := "1200"

  // Carrier configuration
  Carrier_model_path               := model_dir .. "/carrier"
  Carrier_skeleton                 := Carrier_model_path .. "/Carrier.spec"
  Carrier_model_bounds             := Carrier_model_path .. "/carrier.bounds"
  Carrier_model_hull               := Carrier_model_path .. "/carrier-hull-reduced.obj"
  
  // Water rendering configuration
  Water_vertex_shader              := shaders_dir .. "/ocean.vert"
  Water_fragment_shader            := shaders_dir .. "/ocean.frag"
  Water_tile_size                  := "500.0"
  Water_bumpmap                    := texture_dir .. "/water-dudvbump.png"
  Water_tile_uvspan                := "1.0"
  Water_tile_num                   := "51"
) // Config do

