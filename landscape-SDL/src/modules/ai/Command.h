#ifndef AI_COMMAND_H
#define AI_COMMAND_H

#include <landscape.h>
#include <TypedObject.h>

namespace AI {

struct Command : public TypedObject, virtual public Object {
    static struct Types {
        Type Command;
        Type AttackCommand, DefendCommand, PatrolCommand;
        Type TravelCommand, FollowCommand;
        Types();
    } types;

protected:
    inline Command(Type &t) : TypedObject(t) { }
};

struct AttackCommand : public Command {
    Ptr<IActor> target;
    inline AttackCommand(Ptr<IActor> target)
    : Command(types.AttackCommand), target(target)
    { }
};

struct DefendCommand : public Command {
    Ptr<IActor> target;
    inline DefendCommand(Ptr<IActor> target)
    : Command(types.DefendCommand), target(target)
    { }
};

struct PatrolCommand : public Command {
    Vector area;
    float radius;
    inline PatrolCommand(const Vector &v, float r)
    : Command(types.PatrolCommand), area(v), radius(r)
    { }
};

struct TravelCommand : public Command {
    Vector target;
    float radius;
    inline TravelCommand(const Vector &v, float r)
    : Command(types.TravelCommand), target(v), radius(r)
    { }
};

struct FollowCommand : public Command {
    Ptr<IActor> leader;
    inline FollowCommand(Ptr<IActor> leader)
    : Command(types.FollowCommand), leader(leader)
    { }
};

} // namespace Command

#endif
