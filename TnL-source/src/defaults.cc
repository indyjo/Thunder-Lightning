#include <SDL/SDL_keycode.h>
#include <interfaces/IConfig.h>
#include <remap.h>
#include "defaults.h"

void initialize_config(Ptr<IConfig> config)
{
    config->set("terrain_dir", std::string(config->query("data_dir")) + "/lod-terrains/compare");
    config->set("texture_dir", std::string(config->query("data_dir")) + "/textures");
    config->set("model_dir", std::string(config->query("data_dir")) + "/models");
    config->set("fonts_dir", std::string(config->query("data_dir")) + "/fonts");
    config->set("sounds_dir", std::string(config->query("data_dir")) + "/sounds");
    config->set("scripts_dir", std::string(config->query("data_dir")) + "/scripts");
    config->set("shaders_dir", std::string(config->query("data_dir")) + "/shaders");
    config->set("Drone_model_path", std::string(config->query("model_dir")) + "/lightning");

    config->set("Drone_engine_sound", "aircraft-engine-1.wav");
    config->set("Drone_engine_gain_lowest", "0.4");
    config->set("Drone_engine_gain_highest", "1.0");
    config->set("Drone_engine_max_gain", "1.0");
    config->set("Drone_engine_pitch_lowest", "0.7");
    config->set("Drone_engine_pitch_highest", "1.3");
    config->set("Drone_engine_reference_distance", "200.0");
    config->set("Drone_inside_model_file", std::string(config->query("Drone_model_path")) + "/lightning-with-interior.obj");
    config->set("Drone_mfd_model_file", std::string(config->query("Drone_model_path")) + "/lightning-mfd.obj");
    config->set("Drone_model_bounds", std::string(config->query("Drone_model_path")) + "/lightning.bounds");
    config->set("Drone_skeleton", std::string(config->query("Drone_model_path")) + "/lightning.spec");
    config->set("Drone_wheel_model_file", std::string(config->query("model_dir")) + "/misc/tire.obs");
    config->set("Drone_LeftWheel_C_norm", "1000");
    config->set("Drone_LeftWheel_C_tan_min", "50");
    config->set("Drone_LeftWheel_C_tan_max", "1000");
    config->set("Drone_LeftWheel_damping", "15000");
    config->set("Drone_LeftWheel_force", "50000");
    config->set("Drone_LeftWheel_range", "0.8");
    config->set("Drone_NoseWheel_C_norm", "10000");
    config->set("Drone_NoseWheel_C_tan_min", "50");
    config->set("Drone_NoseWheel_C_tan_max", "50");
    config->set("Drone_NoseWheel_damping", "15000");
    config->set("Drone_NoseWheel_force", "100000");
    config->set("Drone_NoseWheel_range", "1.2");
    config->set("Drone_RightWheel_C_norm", "1000");
    config->set("Drone_RightWheel_C_tan_min", "50");
    config->set("Drone_RightWheel_C_tan_max", "1000");
    config->set("Drone_RightWheel_damping", "15000");
    config->set("Drone_RightWheel_force", "50000");
    config->set("Drone_RightWheel_range", "0.8");
    config->set("Environment_clip_min", "1.0");
    config->set("Environment_clip_max", "20000.0");
    config->set("Environment_fog_r", "0.7922");
    config->set("Environment_fog_g", "0.6431");
    config->set("Environment_fog_b", "0.5686");
    config->set("Environment_ground_fog_min", "0.0");
    config->set("Environment_ground_fog_max", "400.0");
    config->set("Environment_ground_fog_range", "3500.0");
    config->set("FontMan_default_font", "dejavu-sans-24-normal");
    config->set("FontMan_dir", config->query("fonts_dir"));
    config->set("Game_info_message_font", "dejavu-sans-24");
    config->set("Game_loading_screen", std::string(config->query("texture_dir")) + "/loading-screen.png");
    config->set("Game_loading_screen_font", "dejavu-sans-16-bold");
    config->set("Game_max_frame_delta", "0.066667");
    config->set("Game_max_ms_for_simulation", "33.333333");
    config->set("Game_max_step_delta", "0.033333");
    config->set("Game_use_shaders", "true");
    config->set("HUD_font_big", "dejavu-sans-20-bold");
    config->set("HUD_font_medium", "dejavu-sans-16-bold");
    config->set("HUD_font_small", "dejavu-sans-10-bold");
    config->set("InfoMessages_realtime", "false");
    config->set("LoDQuadManager_terrain_prefix", std::string(config->query("terrain_dir")) + "/terrain");
    config->set("LoDQuadManager_texture_prefix", std::string(config->query("terrain_dir")) + "/texture");
    config->set("LoDQuadManager_texmap_prefix", std::string(config->query("terrain_dir")) + "/texmap");
    config->set("LoDQuadManager_lightmap_prefix", std::string(config->query("terrain_dir")) + "/lightmap");
    config->set("LoDQuadManager_detail_tex_name", std::string(config->query("texture_dir")) + "/detail-texture.spr");
    config->set("LoDQuadManager_tile_dir", std::string(config->query("texture_dir")) + "/terrain-tiles");
    config->set("LoDQuadManager_texture_list", std::string(config->query("terrain_dir")) + "/textures");
    config->set("LoDQuadManager_quads_w", "1");
    config->set("LoDQuadManager_quads_h", "1");
    config->set("Map_compass_tex", std::string(config->query("texture_dir")) + "/map-compass.png");
    config->set("Map_lines_tex", std::string(config->query("texture_dir")) + "/map-lines.png");
    config->set("Map_f", "6.8");
    config->set("Map_py", "-0.05");
    config->set("Map_y", "3");
    config->set("Map_z", "-6");
    config->set("MFD_font_default", "dejavu-sans-12");
    config->set("MFD_font_small", "dejavu-sans-10-bold");
    config->set("MFD_font_title", "dejavu-sans-12-bold");
    config->set("SkyBox_texture_path", std::string(config->query("texture_dir")) + "/skybox/pastel");
    config->set("SoundMan_sound_dir", config->query("sounds_dir"));
    config->set("TexMan_cache_size", "256");
    config->set("TexMan_missing_texture", std::string(config->query("texture_dir")) + "/missing-texture.png");
    config->set("Water_bumpmap", std::string(config->query("texture_dir")) + "/water-dudvbump.png");
    config->set("Water_fallback_texture", std::string(config->query("texture_dir")) + "/terrain-tiles/water.spr");
    config->set("Water_fragment_shader", std::string(config->query("shaders_dir")) + "/ocean.frag");
    config->set("Water_tile_num", "51");
    config->set("Water_tile_size", "1000.0");
    config->set("Water_tile_uvspan", "2.0");
    config->set("Water_use_shaders", "true");
    config->set("Water_vertex_shader", std::string(config->query("shaders_dir")) + "/ocean.vert");
}

void initialize_input(Ptr<EventRemapper> remapper)
{
    auto registerAction = [&](const char *name, const char *friendly, const char *description)
    {
        remapper->action_dict[name] = EventRemapper::DictionaryEntry(friendly, description);
    };
    auto registerAxis = [&](const char *name, const char *friendly, const char *description)
    {
        remapper->axis_dict[name] = EventRemapper::DictionaryEntry(friendly, description);
    };
    auto mapKey = [&](int key, const char *action)
    {
        remapper->mapButton(EventRemapper::Button(EventRemapper::KEYBOARD_KEY, 0, key), action);
    };
    auto mapMouseButton = [&](int button, const char *action)
    {
        remapper->mapButton(EventRemapper::Button(EventRemapper::MOUSE_BUTTON, 0, button), action);
    };
    auto mapJoystickButton = [&](int device, int button, const char *action)
    {
        remapper->mapButton(EventRemapper::Button(EventRemapper::JOYSTICK_BUTTON, device, button), action);
    };
    auto mapJoystickAxis = [&](int device, int device_axis, const char *axis)
    {
        remapper->mapJoystickAxis(device, device_axis, axis);
    };

    registerAction("mainmenu", "Main menu", "Enter main menu while playing.");
    registerAction("switch-mfd", "Switch MFD mode", "Cycle between the available modes of the multi function display (MFD).");
    registerAction("+forward", "Forward/down", "Accelerate ground-based vehicle. For aircraft, push elevator (nose down).");
    registerAction("+backward", "Backward/brake/up", "Brake and reverse vehicle. For aircraft, pull elevator (nose up).");
    registerAction("+left", "Left", "Steer vehicle left. For aircraft, aileron left (roll counter-clockwise).");
    registerAction("+right", "Right", "Steer vehicle right. For aircraft, aileron right (roll clockwise).");

    registerAction("+rudder_left", "Rudder left (aircraft)", "Rudder sharp left in aircraft. Causes nose to yaw left. Also steers front wheel.");
    registerAction("+rudder_right", "Rudder right (aircraft)", "Rudder sharp right in aircraft. Causes nose to yaw right. Also steers front wheel.");

    registerAction("throttle0", "Throttle 0%", "Sets throttle to 0%.");
    registerAction("throttle1", "Throttle 11%", "Sets throttle to 11%.");
    registerAction("throttle2", "Throttle 22%", "Sets throttle to 22%.");
    registerAction("throttle3", "Throttle 33%", "Sets throttle to 33%.");
    registerAction("throttle4", "Throttle 44%", "Sets throttle to 44%.");
    registerAction("throttle5", "Throttle 55%", "Sets throttle to 55%.");
    registerAction("throttle6", "Throttle 66%", "Sets throttle to 66%.");
    registerAction("throttle7", "Throttle 77%", "Sets throttle to 77%.");
    registerAction("throttle8", "Throttle 88%", "Sets throttle to 88%.");
    registerAction("throttle9", "Throttle 100%", "Sets throttle to 100%.");
    registerAction("increase-throttle", "Throttle: increase by 5%", "");
    registerAction("decrease-throttle", "Throttle: decrease by 5%", "");

    registerAction("autopilot", "Automatic/manual control", "Toggle the current unit's control mode between automatic and manual.");

    registerAction("pause", "Pause", "Toggle game state between paused and unpaused.");
    registerAction("toggle-console", "Console", "Switch console on/off.");

    registerAction("view0", "View 1", "Switch to view 1 (main view) of current unit.");
    registerAction("view1", "View 2", "Switch to view 2 (if available) of current unit.");
    registerAction("view2", "View 3", "Switch to view 3 (if available) of current unit.");
    registerAction("view3", "View 4", "Switch to view 4 (if available) of current unit.");
    registerAction("view4", "View 5", "Switch to view 5 (if available) of current unit.");
    registerAction("view5", "View 6", "Switch to view 6 (if available) of current unit.");

    registerAction("next-view-subject", "Next unit", "Switch to next unit from same faction. Previous unit is set to automatic control.");
    registerAction("external-view", "Observer view", "Switch to/from external observer view.");
    registerAction("cycle-primary", "Weapon", "Cycle between weapons.");
    registerAction("+primary", "Fire", "Fire currently selected weapon.");

    registerAction("debug", "Debug mode", "Toggle debug mode on/off.");

    registerAction("previous-target", "Target previous", "Target previous radar contact.");
    registerAction("next-target", "Target next", "Target next radar contact.");
    registerAction("next-hostile-target", "Target next hostile", "Target next hostile radar contact.");
    registerAction("next-friendly-target", "Target next friendly", "Target next friendly radar contact.");
    registerAction("gunsight-target", "Target object in gunsight", "Target object closest to center of gunsight.");
    registerAction("nearest-target", "Target nearest", "Target nearest radar contact.");
    registerAction("radar-range", "Radar range", "Cycle through available radar display ranges. This does not affect the actual radar range.");

    registerAction("slower", "Time warp: slower", "Decelerate time, causing slow-motion.");
    registerAction("faster", "Time warp: faster", "Accelerate time, causing fast-forward.");

    registerAction("landing-gear", "Landing gear", "Lower/raise landing gear.");
    registerAction("landing-hook", "Landing hook", "Lower/raise landing hook for carrier landings.");
    registerAction("+brake", "Air brake", "Deploy air brake while held.");

    registerAction("+observer-dolly", "Observer: dolly", "While pressed, mouse movements affect observer forward/backward motion.");
    registerAction("+observer-pan", "Observer: pan", "While pressed, mouse movements cause observer left/right and up/down motion.");
    registerAction("observer-stop", "Observer: stop", "Stop the observer immediately.");

    registerAxis("throttle", "Throttle", "Throttle for aircraft and the carrier.");
    registerAxis("aileron", "Aileron", "Aircraft aileron. Controls the aircraft's roll speed.");
    registerAxis("elevator", "Elevator", "Aircraft elevator. Controls the aircraft's pitch speed.");
    registerAxis("rudder", "Rudder", "Aircraft rudder. Controls the aircraft's yaw speed.");
    registerAxis("brake", "Brakes", "Aircraft brakes. Air brakes control speed of descent. Wheel brakes decelerate the aircraft on the ground.");
    registerAxis("car_throttle", "Throttle (vehicle)", "Throttle for land-based vehicles.");
    registerAxis("car_brake", "Brake (vehicle)", "Brake for land-based vehicles.");
    registerAxis("car_steer", "Steering wheel (vehicle)", "Steering for land-based vehicles.");
    registerAxis("turret_steer", "Azimuth (turret)", "Horizontal turret control on tanks and carrier.");
    registerAxis("cannon_steer", "Elevation (turret)", "Vertical turret control on tanks and carrier.");

    mapKey(SDLK_ESCAPE, "mainmenu");
    mapKey(SDLK_TAB, "switch-mfd");

    mapKey(SDLK_UP, "+forward");
    mapKey(SDLK_DOWN, "+backward");
    mapKey(SDLK_LEFT, "+left");
    mapKey(SDLK_RIGHT, "+right");
    mapKey(SDLK_COMMA, "+rudder_left");
    mapKey(SDLK_PERIOD, "+rudder_right");

    mapKey(SDLK_1, "throttle0");
    mapKey(SDLK_2, "throttle1");
    mapKey(SDLK_3, "throttle2");
    mapKey(SDLK_4, "throttle3");
    mapKey(SDLK_5, "throttle4");
    mapKey(SDLK_6, "throttle5");
    mapKey(SDLK_7, "throttle6");
    mapKey(SDLK_8, "throttle7");
    mapKey(SDLK_9, "throttle8");
    mapKey(SDLK_0, "throttle9");
    mapKey(SDLK_a, "autopilot");
    mapKey(SDLK_PLUS, "increase-throttle");
    mapKey(SDLK_MINUS, "decrease-throttle");

    mapKey(SDLK_p, "pause");
    mapKey(SDLK_F11, "toggle-console");

    mapKey(SDLK_F1, "view0");
    mapKey(SDLK_F2, "view1");
    mapKey(SDLK_F3, "view2");
    mapKey(SDLK_F4, "view3");
    mapKey(SDLK_F5, "view4");
    mapKey(SDLK_F6, "view5");

    mapKey(SDLK_F7, "zoom-out");
    mapKey(SDLK_F8, "zoom-in");

    mapKey(SDLK_n, "next-view-subject");
    mapKey(SDLK_v, "external-view");
    mapKey(SDLK_RETURN, "cycle-primary");
    mapKey(SDLK_LCTRL, "+primary");

    mapKey(SDLK_F12, "debug");

    mapKey(SDLK_z, "previous-target");
    mapKey(SDLK_t, "next-target");
    mapKey(SDLK_h, "next-hostile-target");
    mapKey(SDLK_f, "next-friendly-target");
    mapKey(SDLK_g, "gunsight-target");
    mapKey(SDLK_y, "nearest-target");
    mapKey(SDLK_r, "radar-range");

    mapKey(SDLK_F9, "slower");
    mapKey(SDLK_F10, "faster");

    mapKey(SDLK_l, "landing-gear");
    mapKey(SDLK_k, "landing-hook");
    mapKey(SDLK_b, "+brake");

    mapKey(SDLK_i, "toggle-introduction");

    mapMouseButton(1, "+primary");
    mapMouseButton(2, "gunsight-target");
    mapMouseButton(3, "cycle-primary");

    mapMouseButton(1, "+observer-dolly");
    mapMouseButton(3, "+observer-pan");
    mapMouseButton(2, "observer-stop");

    mapJoystickButton(0, 0, "+primary");
    mapJoystickButton(0, 1, "cycle-primary");
    mapJoystickButton(0, 2, "gunsight-target");
    mapJoystickButton(0, 3, "next-friendly-target");
    mapJoystickButton(0, 4, "next-hostile-target");
    mapJoystickButton(0, 5, "+brake");
    mapJoystickButton(0, 7, "landing-gear");
    mapJoystickButton(0, 8, "landing-hook");

    mapJoystickAxis(0, 0, "js_aileron");
    mapJoystickAxis(0, 1, "js_elevator");
    mapJoystickAxis(0, 3, "js_rudder");
    mapJoystickAxis(0, 2, "js_throttle");

    remapper->mapRelativeMouseAxes("mouse_rel_x", "mouse_rel_y");
    remapper->mapAbsoluteMouseAxes("mouse_abs_x", "mouse_abs_y");
}