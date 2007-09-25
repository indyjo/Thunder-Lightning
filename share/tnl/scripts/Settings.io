Settings := Object clone do(
    filename := "config.io"
    
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
    )
    
    load := method(
        file := File clone with(path)
        if (file exists not) then (
            ("Settings file " .. path .. " does not yet exist. Creating it.") println
            save
        )

        if (file isRegularFile not) then(
            ("Not a valid settings file: " .. path) println
        ) else (
            ("Loading user settings from file: " .. path) println
            Config doFile(path)
        )
        
        self
    )
    
    save := method(
        file := File clone with(path) open
        
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
)

// trigger initialization when this file is read
Settings init

