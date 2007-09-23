// This file is executed in a temporary Io context used only during system
// initialization. All variables set here will vanish as soon as the game starts.
// Config settings, however, will persist.

// This is the file executed at the end of system initialization.
// Most config variables, especially those concerning screen resolution,
// are now defined.

"Io: Entering system_init_2.io\n" print

doFile(Config query("data_dir") .. "/scripts/defaults2.io")

"Io: Exiting system_init_1.io\n" print

