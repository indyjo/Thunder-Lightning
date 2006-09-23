path := Config scripts_dir
Importer addSearchPath(path)

Object setSlot("old_doFile", Object getSlot("doFile"))

doFile(path .. "/delegate.io")
doFile(path .. "/Coro.io")
doFile(path .. "/EventTarget.io")
doFile(path .. "/Game.io")
doFile(path .. "/Matrix.io")
doFile(path .. "/Actor.io")
doFile(path .. "/inspect.io")
doFile(path .. "/input.io")
doFile(path .. "/Faction.io")
doFile(path .. "/Completion.io")
doFile(path .. "/Drone.io")
doFile(path .. "/Tank.io")

complete := method(str, context,
  try(
    l:= Completion complete(str, context)
  ) catch (Exception, ex,
    return List clone
  )
  return l
)

rand2 := method(2*(Random value)-1)
randvec := method(vector(rand2,rand2,rand2))

if(Number hasSlot("pi") isNil, Number pi := 2 * 0 acos)
if(Number hasSlot("e") isNil, Number e := 1 exp)

"Loading mission." println
doFile(path .. "/Intro.io")
