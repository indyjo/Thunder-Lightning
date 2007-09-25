// This file is executed in a temporary Io context used only during system
// initialization. All variables set here will vanish as soon as the game starts.
// Config settings, however, will persist.

// This is the file executed at the beginning of system initialization. It is
// responsible for loading most of the configuration settings.
// Some settings are dependent on values determined during system initialization.
// Those can be set in another file, "system_init_2.io"

"Io: Entering system_init_1.io\n" print

# Don't set to above 5.499, there is a bug in the GC!
#Collector setMarksPerAlloc(2)
#Collector setAllocatedStep(1.1)
#Collector setDebug(true)
#Collector collect

doFile(Config query("data_dir") .. "/scripts/convenience.io")
doFile(Config query("data_dir") .. "/scripts/defaults1.io")

# now trigger loading of user settings
doFile(Config query("data_dir") .. "/scripts/Settings.io")
Settings load

"Io: Exiting system_init_1.io\n" print

