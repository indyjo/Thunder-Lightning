// This is the demo that's running when the game hasn't started yet

a := SimpleActor clone
p := TrajectoryPlayer clone with(a, Config query("Game_demo_datafile"))
p is_looping = true
Game setView(a,0)
p start


