#include "mappings.h"

struct Clock;
struct IConfig;

void addMappings(Ptr<IGame> game, IoState * state) {
	addMapping<IActor>(game, state);
	addMapping<IGame>(game, state);
	addMapping<Clock>(game,state);
	addMapping<IConfig>(game,state);
}
