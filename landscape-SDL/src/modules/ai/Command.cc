#include "Command.h"

namespace AI {

Command::Types Command::types;

Command::Types::Types()
:   Command("Command"),
    AttackCommand("AttackCommand", Command),
    DefendCommand("DefendCommand", Command),
    PatrolCommand("PatrolCommand", Command),
    TravelCommand("TravelCommand", Command),
    FollowCommand("FollowCommand", Command)
{ }


} // namespace AI
