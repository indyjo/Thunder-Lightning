"Io: Entering init.io\n" print

# Don't set to above 5.499, there is a bug in the GC!
Collector setMarksPerAlloc(2)


Collector setAllocatedStep(1.1)
#Collector setDebug(true)

#Collector collect

doFile(Config query("base_dir") .. "/share/landscape/scripts/config.io")
AddonLoader appendSearchPath( (Config base_dir) .. "/lib/io/addons")

"Io: Exiting init.io\n" print

