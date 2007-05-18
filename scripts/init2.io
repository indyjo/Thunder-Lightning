"Io: Entering init2.io\n" print

path := Config scripts_dir
Importer addSearchPath(path)

if(Number hasSlot("pi") not, Number pi := 2 * 0 acos)
if(Number hasSlot("e") not, Number e := 1 exp)

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
doFile(path .. "/Matrix.io")
doFile(path .. "/Actor.io")
doFile(path .. "/inspect.io")
doFile(path .. "/Faction.io")
doFile(path .. "/Completion.io")
doFile(path .. "/Drone.io")
doFile(path .. "/Tank.io")
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

rand2 := method(2*(Random value)-1)
randvec := method(vector(rand2,rand2,rand2))

"Io: Exiting init2.io\n" print
