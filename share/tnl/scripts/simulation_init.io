"Io: Entering simulation_init.io\n" print

// Enable easy access to Config
doFile(Config query("data_dir") .. "/scripts/convenience.io")

AddonLoader appendSearchPath( (Config base_dir) .. "/lib/io/addons")

path := Config scripts_dir
Importer addSearchPath(path)

if(Number hasSlot("pi") not, Number pi := 2 * 0 acos)
if(Number hasSlot("e") not, Number e := 1 exp)
Object rand2 := method(2*(Random value)-1)
Object randvec := method(vector(rand2,rand2,rand2))


Object oldDoFile := Object getSlot("doFile")
Object doFile := method(file,
  ("Executing " .. file .. ".") println
  result := call delegateToMethod(self, "oldDoFile")
  ("Done with " .. file .. ".") println
  getSlot("result")
)
doFile(path .. "/input.io")
doFile(path .. "/delegate.io")
doFile(path .. "/Coro.io")
doFile(path .. "/EventTarget.io")
doFile(path .. "/Game.io")
doFile(path .. "/inspect.io")
doFile(path .. "/Matrix.io")
doFile(path .. "/Actor.io")
doFile(path .. "/Faction.io")
doFile(path .. "/Completion.io")
doFile(path .. "/Drone.io")
doFile(path .. "/Tank.io")
doFile(path .. "/Carrier.io")
doFile(path .. "/DataNode.io")
doFile(path .. "/Objective.io")
doFile(path .. "/Mission.io")

complete := method(str, context,
  l := list("ErrorWhileCompleting")
  ex := try(
    l := Completion complete(str, context)
  )
  
  ex catch(Exception,
    ex println
    ex showStack println
  )
  
  return l
)

// force a garbage collection more or less every minute
memoryWatchdog := Object
memoryWatchdog run := method(
    loop(
        (60*25) repeat(yield)
        n := Collector collect
        "Memory watchdog collected #{n} objects." interpolate println
    )
)
memoryWatchdog @@run

Settings loadControls

"Io: Exiting simulation_init.io\n" print

