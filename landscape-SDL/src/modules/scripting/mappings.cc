#include "mappings.h"

struct Clock;
struct IConfig;
struct IActorStage;
struct IPositionProvider;
struct IMovementProvider;
struct IPositionReceiver;
struct IMovementReceiver;
struct Faction;
struct Drone;
struct Tank;
struct Environment;

void addBasicMappings(Ptr<IGame> game, IoState * state) {
	addMapping<IGame>(game, state);
	addMapping<IConfig>(game,state);
}

void addMappings(Ptr<IGame> game, IoState * state) {
	addMapping<Clock>(game,state);
	addMapping<IActor>(game, state);
	addMapping<EventRemapper>(game,state);
	addMapping<IPositionProvider>(game,state);
	addMapping<IMovementProvider>(game,state);
	addMapping<IPositionReceiver>(game,state);
	addMapping<IMovementReceiver>(game,state);
	addMapping<IActorStage>(game,state);
	addMapping<Faction>(game,state);
	addMapping<Drone>(game,state);
	addMapping<Tank>(game,state);
	addMapping<Environment>(game,state);
}
