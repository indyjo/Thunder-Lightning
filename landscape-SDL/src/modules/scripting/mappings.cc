#include "mappings.h"

struct Clock;
struct IConfig;
struct IPositionProvider;

void addBasicMappings(Ptr<IGame> game, IoState * state) {
	addMapping<IGame>(game, state);
	addMapping<Clock>(game,state);
	addMapping<IConfig>(game,state);
}

void addMappings(Ptr<IGame> game, IoState * state) {
	addMapping<IActor>(game, state);
	addMapping<EventRemapper>(game,state);
	addMapping<IPositionProvider>(game,state);
}
