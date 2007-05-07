"Io: Entering init.io\n" print

Collector setDebug(true)
//Collector setSweepsPerGeneration(50)
//Importer turnOff

doFile(Config query("base_dir") .. "/share/landscape/scripts/config.io")
AddonLoader appendSearchPath( (Config base_dir) .. "/lib/io/addons")

"Io: Exiting init.io\n" print

