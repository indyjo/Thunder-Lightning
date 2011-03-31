#include "Missile.h"

class DumbMissile: public Missile
{
public:
    DumbMissile(Ptr<IGame> thegame, Ptr<IActor> target, Ptr<IActor> source=0);
};
