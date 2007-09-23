// This configuration file defines global config values for the game. It does not,
// however, include values intended for the user to edit (like screen resolution
// etc). These entries are defined in config.io.

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
  Game_enable_SDL_parachute     := "true"
  Game_loading_screen           := texture_dir .. "/loading-screen.png"
  //Game_max_step_delta           := (1/30) asString
  Game_max_step_delta           := (1/30) asString
  Game_max_frame_delta          := (1/15) asString
  Game_max_ms_for_simulation    := (1000/30) floor asString
  Game_cegui_dir                := data_dir .. "/cegui/"
  Game_cegui_schemes_dir        := Game_cegui_dir .. "schemes/"
  Game_cegui_imagesets_dir      := Game_cegui_dir .. "imagesets/"
  Game_cegui_fonts_dir          := Game_cegui_dir .. "fonts/"
  Game_cegui_layouts_dir        := Game_cegui_dir .. "layouts/"
  Game_cegui_looknfeels_dir     := Game_cegui_dir .. "looknfeel/"

  // Fonts config
  Console_font              := "dejavu-sans-mono-14"
  Game_info_message_font    := "dejavu-sans-24"
  Game_loading_screen_font  := "dejavu-sans-16-bold"
  MFD_font_default          := "dejavu-sans-12"
  MFD_font_title            := "dejavu-sans-12-bold"
  MFD_font_small            := "dejavu-sans-10-bold"
  HUD_font_small            := "dejavu-sans-10"
  HUD_font_medium           := "dejavu-sans-16-bold"
  HUD_font_big              := "dejavu-sans-20-bold"

  // Texman config
  TexMan_cache_size         := "256"
  TexMan_missing_texture    := texture_dir .. "/missing-texture.png"

  // Environment config
  Environment_clip_min      := "1.0"
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

  // Sky (clouds layer) configuration
  Sky_upside_texture_path          := texture_dir .. "/sky-layer1-above.spr"
  Sky_downside_texture_path        := texture_dir .. "/sky-layer1-below.spr"

  // SkyBox configuration
  SkyBox_texture_path              := texture_dir .. "/skybox/pastel"

  // Gunsight configuration
  Gunsight_texture_file            := texture_dir .. "/gunsight.spr"

  // SmokeTrail configuration
  SmokeTrail_smoke_tex             := texture_dir .. "/missile_smoke.spr"
  SmokeTrail_puffy_tex             := texture_dir .. "/puffy_smoke.png"

  // SmokeColumn configuration
  SmokeColumn_puffy_tex            := texture_dir .. "/puffy_smoke.png"

  // Explosion configuration
  Explosion_frames                 := "16"
  Explosion_seconds_per_frame      := "0.08"
  Explosion_tex_prefix             := texture_dir .. "/explosion/explosion"
  Explosion_tex_postfix            := ".png"
  Explosion_size                   := "20.0"
  Explosion_num_sounds             := "1"
  Explosion_sound_prefix           := "explosion-0"
  Explosion_sound_postfix          := ".wav"

  // Drone configuration
  Drone_model_path                 := model_dir .. "/lightning"
  Drone_skeleton                   := Drone_model_path .. "/lightning.spec"
  Drone_inside_model_file          := Drone_model_path .. "/lightning-with-interior.obj"
  Drone_outside_model_file         := Drone_model_path .. "/lightning.obj"
  Drone_model_bounds               := Drone_model_path .. "/lightning.bounds"
  Drone_mfd_model_file             := Drone_model_path .. "/lightning-mfd.obj"
  Drone_target_radius              := "3.5"
  Drone_pilot_pos                  := "Vector3( 0 0.8 1.9)"
  Drone_cannon_sound               := "cannon-shoot-1.wav"
  Drone_engine_sound               := "aircraft-engine-1.wav"
  Drone_engine_gain_lowest         := "0.4"
  Drone_engine_gain_highest        := "1.0"
  Drone_engine_pitch_lowest        := "0.7"
  Drone_engine_pitch_highest       := "1.3"
  Drone_engine_max_gain            := "1.0"
  Drone_engine_reference_distance  := "200.0"
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
  
  Drone_Vulcan_rounds              := "250"
  Drone_Sidewinder_rounds          := "6"
  Drone_Hydra_rounds               := "14"

  // Tank configuration
  Tank_model_path                  := model_dir .. "/newtank"
  Tank_skeleton                    := Tank_model_path .. "/Tank.spec"
  Tank_model_bounds                := Tank_model_path .. "/Tank.bounds"
  Tank_cannon_sound                := "cannon-shoot-1.wav"
  Tank_engine_sound_low            := "car-engine-1-loop.wav"
  Tank_engine_sound_high           := "car-engine-2-loop.wav"
  Tank_cannon_rounds               := "25"
  Tank_cannon_factor               := "10"
  Tank_vulcan_rounds               := "1200"
  Tank_vulcan_factor               := "0.5"
  Tank_vulcan_loadtime             := (2/5) asString

  // FontMan configuration
  FontMan_dir                      := fonts_dir
  FontMan_default_font             := "dejavu-sans-24-normal"

  // Io Scripting configuration
  Io_init_script                   := scripts_dir .. "/init.io"
  Io_init_script_2                 := scripts_dir .. "/init2.io"
  
  // Missile configurations
  DumbMissile_model                := model_dir .. "/missiles/dumbmissile.obj"
  DumbMissile_engine_sound         := "rocket-engine-1.wav"
  DumbMissile_mass                 := "10.705"
  DumbMissile_Ixx                  := "6"
  DumbMissile_Iyy                  := "6"
  DumbMissile_Izz                  := "2"
  DumbMissile_CdA_f                := (0.02 * Number constants pi * 0.034925 squared) asString
  DumbMissile_CdA_s                := (1.2 * (2*0.034925*0.75 + 0.02)) asString
  DumbMissile_torque_factor_z      := "3"
  DumbMissile_torque_factor_xy     := "3"
  DumbMissile_pitching_factor      := "0.1"
  DumbMissile_blast_begin          := "0"
  DumbMissile_blast_end            := "1.05"
  DumbMissile_max_thrust           := "7545"
  DumbMissile_min_explosion_age    := "0"
  DumbMissile_max_lifetime         := "30"
  
  SmartMissile_model               := model_dir .. "/missiles/smartmissile.obj"
  SmartMissile_engine_sound        := "rocket-engine-1.wav"
  SmartMissile_mass                := "85.5"
  SmartMissile_Ixx                 := "60"
  SmartMissile_Iyy                 := "60"
  SmartMissile_Izz                 := "20"
  SmartMissile_torque_factor_z     := "100"
  SmartMissile_torque_factor_xy    := "100"
  SmartMissile_pitching_factor     := "0"
  // The CdA values were calculated from unmodified previous guesswork :-/
  SmartMissile_CdA_f               := (0.02 * Number constants pi * 0.065 squared) asString
  SmartMissile_CdA_s               := (1.2 * 2.85*2*0.065) asString
  SmartMissile_blast_begin         := "0.3"
  SmartMissile_blast_end           := "1.55"
  SmartMissile_max_thrust          := "40000"
  SmartMissile_min_explosion_age   := "1.5"
  SmartMissile_max_lifetime        := "75"
  // PID constants for the smart missile's homing controller
  //SmartMissile_Kp                  := "1"
  //SmartMissile_Ki                  := "0"
  //SmartMissile_Kd                  := "5"
  SmartMissile_Kp                  := "8"
  SmartMissile_Ki                  := "0"
  SmartMissile_Kd                  := "20"
  
  // Weapons configuration
  Cannon_loadtime                  := "2"
  Cannon_singleshot                := "true"
  Cannon_guided                    := "false"
  Cannon_max_range                 := "8000"
  Cannon_time_of_accel             := "0"
  Cannon_factor                    := "10"
  Cannon_muzzle_velocity           := "1200"
  Cannon_reference_speed           := Cannon_muzzle_velocity
  
  Cannon_sound                     := "cannon-shoot-1.wav"
  Cannon_reference_distance        := "200.0"
  Cannon_gain                      := "1.0"
  
  Vulcan_factor                    := "0.45"
  Vulcan_loadtime                  := (1/5) asString
  Vulcan_singleshot                := "false"
  Vulcan_guided                    := "false"
  
  Vulcan_max_range                 := "5000"
  Vulcan_muzzle_velocity           := "800"
  Vulcan_reference_speed           := Vulcan_muzzle_velocity
  Vulcan_time_of_accel             := "0"

  Vulcan_sound                     := "cannon-shoot-2.wav"
  Vulcan_reference_distance        := "100.0"
  Vulcan_gain                      := "0.05"
  
  Hydra_loadtime                   := "1"
  Hydra_singleshot                 := "true"
  Hydra_guided                     := "false"
  Hydra_max_range                  := "5000"
  Hydra_time_of_accel              := "1.05"
  Hydra_reference_speed            := "739.14"
  Hydra_launch_speed               := "0"
  Hydra_needs_target               := "false"

  Sidewinder_loadtime              := "1"
  Sidewinder_singleshot            := "true"
  Sidewinder_guided                := "true"
  Sidewinder_max_range             := "12000"
  Sidewinder_time_of_accel         := "0.0"
  Sidewinder_reference_speed       := "0.0"
  Sidewinder_launch_speed          := "0"
  Sidewinder_needs_target          := "true"

  Decoy_loadtime                   := "0.7"
  Decoy_singleshot                 := "true"
  Decoy_guided                     := "false"
  Decoy_max_range                  := "0"
  Decoy_time_of_accel              := "0"
  Decoy_reference_speed            := "1.0"
  Decoy_launch_speed               := "0"
  Decoy_needs_target               := "false"
  
  // Carrier configuration
  Carrier_model_path               := model_dir .. "/carrier"
  Carrier_skeleton                 := Carrier_model_path .. "/Carrier.spec"
  Carrier_model_bounds             := Carrier_model_path .. "/carrier.bounds"
  Carrier_model_hull               := Carrier_model_path .. "/carrier-hull-reduced.obj"
  Carrier_Vulcan_rounds            := "2400"
  
  // Water rendering configuration
  Water_vertex_shader              := shaders_dir .. "/ocean.vert"
  Water_fragment_shader            := shaders_dir .. "/ocean.frag"
  Water_tile_size                  := "1000.0"
  Water_bumpmap                    := texture_dir .. "/water-dudvbump.png"
  Water_tile_uvspan                := "2.0"
  Water_tile_num                   := "51"
  Water_fallback_texture           := texture_dir .. "/terrain-tiles/water.spr"
  
  // Observer configuration
  Observer_accel_x                 := "10"
  Observer_accel_y                 := "-10"
  Observer_accel_z                 := "-15"
  Observer_rotate_x                := "0.2"
  Observer_rotate_y                := "0.2"
  Observer_speed_factor            := "0.04"
  
  // Lensflare configuration
  Lensflare_reflex_tex             := texture_dir .. "/lensreflex.png"
  Lensflare_sparkle_thin_tex       := texture_dir .. "/sparkle-thin.png"
  Lensflare_sparkle_thick_tex      := texture_dir .. "/sparkle-thick.png"
  Lensflare_rot_speed_thin         := "-0.5"
  Lensflare_rot_speed_thick        := "1.5"
  
  // Info messages
  InfoMessages_realtime            := "false"

) // Config do
