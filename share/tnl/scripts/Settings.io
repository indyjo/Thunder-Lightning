File hasSlot("openForWriting") ifFalse(
    File openForWriting := method(optPath,
        optPath ifNonNil( setPath(optPath) )
        if (exists,
            truncateToSize(0)
        )
        openForAppending
    )
)

Settings := Object clone do(
    filename := "config.io"
    controls_filename := "controls.io"
    
    init := method(
        if (System platform asLowercase == "windows") then(
            self dir := Config base_dir
        ) else (
            self dir := System getenv("HOME") .. "/.tnl"
        )
        
        # Make sure the settings dir exists and is a directory
        if (File clone with(dir) exists) then(
            File clone with(dir) isDirectory ifFalse(
                Exception raise("Can't initialize settings: " .. dir .. " is not a directory")
            )
        ) else (
            ("Settings directory " .. dir .. " does not yet exist. Creating it.") println
            Directory clone with(dir) create
        )
        
        self path := dir .. "/" .. filename
        self controls_path := dir .. "/" .. controls_filename
    )
    
    loadConfig := method(
        file := File clone with(path)
        if (file exists not) then (
            ("Settings file " .. path .. " does not yet exist. Creating it.") println
            saveConfig
        )

        if (file isRegularFile not) then(
            ("Not a valid settings file: " .. path) println
        ) else (
            ("Loading user settings from file: " .. path) println
            Config doFile(path)
        )
        
        self
    )

    saveConfig := method(
        file := File clone openForWriting(path)
        
        file write("# This is where Thunder&Lightning saves its user settings.\n")
        file write("\n")
        
        list(
            "Game_fullscreen",
            "Game_auto_resolution",
            "Game_xres",
            "Game_yres",
            "Game_red_bits",
            "Game_green_bits",
            "Game_blue_bits",
            "Game_zbuffer_bits",
            "Game_fsaa_enabled",
            "Game_fsaa_samples",
            "Game_use_shaders",
            "Water_mirror_texture_size",
            "Water_use_shaders",
            "Controls_enable_joystick",
            "Controls_enable_mouse",
            "Controls_joystick_sensitivity"
        ) foreach(key,
            file write("#{key} := \"#{Config query(key)}\"\n" interpolate)
        )
        
        file close
        self
    )
    
    loadControls := method(
        file := File clone with(controls_path)
        if (file exists not) then (
            ("Settings file " .. controls_path .. " does not yet exist. Creating it.") println
            saveControls
        )

        if (file isRegularFile not) then(
            ("Not a valid settings file: " .. controls_path) println
        ) else (
            ("Loading user settings from file: " .. controls_path) println
            EventRemapper clearButtonMappings
            EventRemapper clearJoystickAxisMappings
            EventRemapper doFile(controls_path)
        )
        
        self
    )
    
    saveControls := method(
        file := File clone openForWriting(controls_path)
        
        file write("# This is where Thunder&Lightning saves its controls configuration.\n\n")
        
        EventRemapper actions foreach(action,
            name := action name
            action buttons foreach(button,
                if (button type == "KEYBOARD_KEY") then(
                    file write("mapKey(#{EventRemapper keySymOf(button button)}, \"#{name}\")\n" interpolate)
                ) elseif (button type == "MOUSE_BUTTON") then(
                    file write("mapMouseButton(#{button button}, \"#{name}\")\n" interpolate)
                ) elseif (button type == "JOYSTICK_BUTTON") then(
                    file write("mapJoystickButton(#{button device}, #{button button}, \"#{name}\")\n" interpolate)
                )
            )
        )
        
        EventRemapper registeredAxes foreach(axis,
            // We have to query for "+js_axis" and "-js_axis"
            EventRemapper joystickAxesForAxis("+js_" .. axis) foreach(joyaxis,
                file write("mapJoystickAxis(#{joyaxis joystick}, #{joyaxis axis}, \"+js_#{axis}\")\n" interpolate)
            )
            EventRemapper joystickAxesForAxis("-js_" .. axis) foreach(joyaxis,
                file write("mapJoystickAxis(#{joyaxis joystick}, #{joyaxis axis}, \"-js_#{axis}\")\n" interpolate)
            )
        )
        
        file write("\n")
        file close
        self
    )
)

// trigger initialization when this file is read
Settings init

